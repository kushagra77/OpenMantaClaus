"""Detection filtering and feature observation generation.

This module contains the DetectionFilter class responsible for:
- Filtering raw YOLO predictions based on confidence and task-specific rules
- Transforming bounding boxes from model space back to image space
- Converting detections to FeatureObservations for downstream SLAM/task pipelines
- Tracking current task state and task-phase information
"""

import math
from typing import Dict, List, Optional, Sequence, Tuple

import cv2
import numpy as np
from interfaces.msg import FeatureObservation, FeatureObservations
# color constants, same as tasks
RED = 1
BLUE = 2
YELLOW = 3
UNKNOWN = 0

# Class name and color mappings for visualization
CLASS_NAMES = {
    0: "flag",
    1: "gate",
    2: "flare",
    3: "bucket",
}

CLASS_COLORS = {
    0: (255, 0, 0),
    1: (0, 255, 0),
    2: (0, 165, 255),
    3: (255, 0, 255),
}


class DetectionFilter:
    """Handles detection filtering, transformation, and feature observation generation.

    Attributes:
        _camera_fx (float): Camera focal length in x (pixels).
        _camera_cx (float): Camera principal point x coordinate (pixels).
        _feature_id_by_name (Dict[str, int]): Map from feature name to canonical ID.
        _conf_threshold (float): Confidence threshold for filtering detections.
        _cur_task (str): Current task name (e.g., "flag", "gate", "bucket").
        _task_initial (bool): Whether the current task is in its initial phase.
    """

    def __init__(
        self,
        camera_fx: float,
        camera_cx: float,
        feature_names: List[str],
        red_lower: List[int],
        red_higher: List[int],
        blue_lower: List[int], # only for bucket, not flare
        blue_higher: List[int],
        yellow_lower: List[int],
        yellow_higher: List[int],
        erode_kernel_size: int,
        flare_percentage_threshold: float,
        bucket_percentage_threshold: float,
        conf_threshold: float = 0.8,
        curr_task: str = "gate",
        base_std: float = math.radians(0.14),
    ):
        self._camera_fx = camera_fx
        self._camera_cx = camera_cx
        self._conf_threshold = conf_threshold
        self._feature_id_by_name = {name: index for index, name in enumerate(feature_names)}
        self._cur_task = curr_task
        self._task_initial = True
        self._latest_raw_image: Optional[np.ndarray] = None
        self.detections: Optional[List[dict]] = []
        
        # HSV color ranges
        self.red_lower = red_lower
        self.red_higher = red_higher
        self.blue_lower = blue_lower
        self.blue_higher = blue_higher
        self.yellow_lower = yellow_lower
        self.yellow_higher = yellow_higher
        
        self.flare_percentage_threshold = flare_percentage_threshold
        self.bucket_percentage_threshold = bucket_percentage_threshold
        self.erode_kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (erode_kernel_size, erode_kernel_size))
        self.base_std = base_std

    def set_latest_raw_image(self, image: np.ndarray) -> None:
        self._latest_raw_image = image

    def set_task_state(self, task_name: str, initial: bool) -> None:
        self._cur_task = task_name
        self._task_initial = initial

    def get_task_state(self) -> Tuple[str, bool]:
        return self._cur_task, self._task_initial

    def _get_obs_from_x(self, x: float) -> FeatureObservation:
        """Convert an image-space x coordinate to a `FeatureObservation`.

        Uses the calibrated focal length and principal point to compute a
        bearing (radians) for the EKF SLAM pipeline.
        """
        obs = FeatureObservation()
        obs.bearing = self.get_angle_from_x(x)
        return obs

    def get_angle_from_x(self, x: float) -> float:
        return float(math.atan2(self._camera_cx - x, self._camera_fx))

    def get_mask(self, frame, color):
        if color == RED:
            lower = np.array(self.red_lower)
            upper = np.array(self.red_higher)
        elif color == BLUE:
            lower = np.array(self.blue_lower)
            upper = np.array(self.blue_higher)
        elif color == YELLOW:
            lower = np.array(self.yellow_lower)
            upper = np.array(self.yellow_higher)
        else:
            raise ValueError(f"Unknown color: {color}")
        
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        mask = cv2.inRange(hsv, lower, upper)
        
        # add a little bit of lower red range
        if (color == RED):
            mask = cv2.bitwise_or(mask, cv2.inRange(hsv, np.array([0, lower[1], lower[2]]), np.array([15, upper[1], upper[2]])))
        
        # hsv is strict so dilate to add more  
        mask = cv2.dilate(mask, self.erode_kernel, iterations=1)
        return mask
    
    def get_flare_color(self, frame):
        red_mask = self.get_mask(frame, RED)
        yellow_mask = self.get_mask(frame, YELLOW)
        
        red_count = cv2.countNonZero(red_mask)
        yellow_count = cv2.countNonZero(yellow_mask)
        all_count = frame.shape[0] * frame.shape[1]
        final_color = UNKNOWN
        if (red_count / all_count) > self.flare_percentage_threshold:
            if red_count > yellow_count*1.5:
                final_color = RED
        
        if (yellow_count / all_count) > self.flare_percentage_threshold:
            if yellow_count > red_count*1.5:
                final_color = YELLOW

        return final_color
    
    def get_bucket_color(self, frame):
        red_mask = self.get_mask(frame, RED)
        blue_mask = self.get_mask(frame, BLUE)
        
        red_count = cv2.countNonZero(red_mask)
        blue_count = cv2.countNonZero(blue_mask)
        all_count = frame.shape[0] * frame.shape[1]
        final_color = UNKNOWN
        
        if (red_count / all_count) > self.bucket_percentage_threshold:
            if red_count > blue_count*2.0:
                final_color = RED
        
        if (blue_count / all_count) > self.bucket_percentage_threshold:
            if blue_count > red_count*2.0:
                final_color = BLUE

        return final_color
    
    def filter_flag(self, bboxes_with_conf: List[dict]) -> FeatureObservations:
        ''' 
        ignore any boxes with confidence below 0.85 
        and those with area smaller than 3000 pixels (to filter out small false positives)
        and thos with height less than half the image height
        '''
        img_h, img_w = self._latest_raw_image.shape[:2]
        observations = []
        for det in bboxes_with_conf:
            if det["conf"] < 0.85:
                break
            if det["w"] * det["h"] < 3000:
                continue
            if det["h"] < self._latest_raw_image.shape[0] * 0.5:
                continue
            
            obs = self._get_obs_from_x(self.get_centroid(det)[0])
            obs.confident = False
            obs.id = self._feature_id_by_name.get("flag", -1)
            
            # get std based on box width
            bearing_std = self.base_std * ((img_w / 6.0) / max(det["w"], 1.0))
            obs.bearing_cov = bearing_std**2  # Set a default covariance value
            observations.append(obs)
            break # Only take the most confident valid gate detection
            
        return observations 
            

    def filter_gate(self, bboxes_with_conf: List[dict], feature="gate", initial=True) -> FeatureObservations:
        ''' 
        ignore any boxes with confidence below 0.85
        and those that are too rectangular
        and those whose width is really small (<10% of image width)
        and thos that are too close to the ends of the image (5%)
        for qual_gate, also ignore those that are too far from the center (45% of image width)
        '''
        
        img_h, img_w = self._latest_raw_image.shape[:2]
        observations = []
        chosen_count = 0
        centroid = -1000
        for det in bboxes_with_conf:
            if det["conf"] < 0.75:
                break
            aspect_ratio = det["w"] / det["h"] if det["h"] > 0 else 0
            if aspect_ratio < 0.5 or aspect_ratio > 2.0:
                continue
            if det["w"] < img_w*0.1:
                continue
            if det["x"] < img_w*0.05 or (det["x"] + det["w"]) > img_w*0.95:
                continue
            if feature == "qual_gate" and np.abs(det["x"] + det["w"]*0.5 - img_w*0.5) > img_w*0.45:
                continue

            left_obs = self._get_obs_from_x(det["x"] + det["w"]*0.05)
            right_obs = self._get_obs_from_x(det["x"] + det["w"]*0.95)
            if not initial:
                left_obs, right_obs = right_obs, left_obs
            cur_centroid = self.get_angle_from_x(det["x"] + det["w"]*0.5)
            
            chosen_count += 1    
            bearing_std = self.base_std * ((img_w/3.0) / max(det["w"], 1.0))
            left_obs.bearing_cov = bearing_std**2
            right_obs.bearing_cov = bearing_std**2

            observations.append(left_obs)
            observations.append(right_obs)
            
            left_obs.id = self._feature_id_by_name.get("gate_left", -1)
            right_obs.id = self._feature_id_by_name.get("gate_right", -1)
            if feature == "qual_gate":
                left_obs.id = self._feature_id_by_name.get("qual_gate_left", -1)
                right_obs.id = self._feature_id_by_name.get("qual_gate_right", -1)
                if (chosen_count == 2):
                    if (math.fabs(cur_centroid) < math.fabs(centroid)):
                        observations = observations[-2:]  # keep the most centered gate
                    else:
                        observations = observations[:-2]
                    break
                else:
                    centroid = cur_centroid
            else:
                break # Only take the most confident valid main gate detection
            
        # account for partial detections
        for obs in observations:
            if obs.bearing > self.get_angle_from_x(img_w*0.05) or obs.bearing < self.get_angle_from_x(img_w*0.95):
                observations.remove(obs)
        return observations 


    def filter_flare(self, bboxes_with_conf: List[dict]) -> FeatureObservations:
        '''
        ignore any boxes with confidence below 0.75
        and centroid in the top third of the image
        and any boxes that are not vertical rectangles
        mark all boxes as flare_1
        '''
        img_h, img_w = self._latest_raw_image.shape[:2]
        flares = []
        for det in bboxes_with_conf:
            if det["conf"] < 0.75:
                break
            center_x, center_y = self.get_centroid(det)
            if center_y < img_h / 3:
                continue
            aspect_ratio = det["w"] / det["h"] if det["h"] > 0 else 0
            if aspect_ratio > 0.35:
                continue

            obs = self._get_obs_from_x(center_x)
            obs.confident = False
            obs.id = self._feature_id_by_name.get("flare_1", -1)
            bearing_std = self.base_std * ((img_w/6.0) / max(det["w"], 1.0))
            obs.bearing_cov = bearing_std**2  # Set covariance based on width
            flares.append((obs, det))
            
        flares = flares[:3]  # Only take up to 3 flares with highest confidence
        seen = [False, False, False, False]  # Track if we've assigned colors to flares
        observations = []
        for obs, det in flares:
            cropped_flare_image = self._latest_raw_image[int(det["y"]):int(det["y"]+det["h"]), int(det["x"]):int(det["x"]+det["w"])]
            obs.color = self.get_flare_color(cropped_flare_image)
            if (seen[obs.color]):
                obs.color = UNKNOWN 
                
            seen[obs.color] = True
            observations.append(obs)
        
        return observations

    def filter_bucket(self, bboxes_with_conf: List[dict]) -> FeatureObservations:
        '''
        ignore any boxes with confidence below 0.75
        and with centroid in the top third of the image
        and area less than 1000 pixels
        and y coordinate not within 50px of most confident detection
        and those that are too rectangular        
        and those with width smaller than 5% of image width
        and area not within 40% of most confident detection
        mark bucket ids from 1-4, left to right, return observations in right to left order
        '''
        img_h, img_w = self._latest_raw_image.shape[:2]
        buckets = []
        count = 0
        most_confident_center_y = -1
        most_confident_area = -1 
        bearing_std = -1
        for det in bboxes_with_conf:
            center_x, center_y = self.get_centroid(det)
            if det["conf"] < 0.75:
                break
            if center_y < img_h / 3:
                continue
            if det["w"] * det["h"] < 1000:
                continue
            aspect_ratio = det["w"] / det["h"] if det["h"] > 0 else 0
            if aspect_ratio < 0.3 or aspect_ratio > 3.0:
                continue
            if det["w"] < img_w * 0.05: # wrong or too far
                continue
            
            obs = self._get_obs_from_x(center_x)
            if count == 0:
                most_confident_center_y = center_y
                most_confident_area = det["w"] * det["h"]
                bearing_std = self.base_std * ((img_w/4.0) / max(det["w"], 1.0))
            else:
                if abs(center_y - most_confident_center_y) > 50:
                    continue
                area = det["w"] * det["h"]
                if abs(area - most_confident_area) / most_confident_area > 0.4:
                    continue
            obs.confident = False  # never confident really
            buckets.append((obs, det))
            count += 1
            if count == 4:
                break
        
        buckets = sorted(buckets, key=lambda obs: obs[0].bearing)  # Sort right to left
        cov = (bearing_std*(1+(4-count) * 3))**2 # less buckets = more uncertainty
        observations = []
        blue_seen = False
        for i, (obs, det) in enumerate(buckets):
            obs.id = self._feature_id_by_name.get(f"bucket_{count - i}", -1)
            obs.bearing_cov = cov
            cropped_bucket_image = self._latest_raw_image[int(det["y"]):int(det["y"]+det["h"]), int(det["x"]):int(det["x"]+det["w"])]
            obs.color = self.get_bucket_color(cropped_bucket_image)
            if blue_seen and obs.color == BLUE:
                obs.color = UNKNOWN
            elif obs.color == BLUE:
                blue_seen = True
            observations.append(obs)
 
        return observations

    def get_centroid(self, det: dict) -> Tuple[float, float]:
        x = det["x"]
        y = det["y"]
        w = det["w"]
        h = det["h"]
        center_x = x + (w / 2.0)
        center_y = y + (h / 2.0)
        return center_x, center_y
    
    def _detections_to_observations(
        self
    ) -> FeatureObservations:
        feature_msg = FeatureObservations()
        # feature_msg.header.stamp = self.get_clock().now().to_msg()
        feature_msg.header.frame_id = "manta_camera"
        observations: List[FeatureObservation] = []
        gates = []
        flares = []
        buckets = []
        flags = []
        detections = sorted(self.detections, key=lambda d: d["conf"], reverse=True)  # Sort by confidence descending
        for det in detections:
            if CLASS_NAMES.get(int(det["cls"]), "unknown") == "gate":
                gates.append(det)
            elif CLASS_NAMES.get(int(det["cls"]), "unknown") == "flare":
                flares.append(det)
            elif CLASS_NAMES.get(int(det["cls"]), "unknown") == "bucket":
                buckets.append(det)
            elif CLASS_NAMES.get(int(det["cls"]), "unknown") == "flag":
                flags.append(det)
        
        if self._cur_task == "gate":
            observations.extend(self.filter_gate(gates, feature="gate", initial=self._task_initial))
            observations.extend(self.filter_flag(flags))
            observations.extend(self.filter_flare(flares))
        elif self._cur_task == "qual_gate":
            observations.extend(self.filter_gate(gates, feature="qual_gate", initial=self._task_initial))
        elif self._cur_task == "flare":
            # might add more later?
            observations.extend(self.filter_flag(flags))
            observations.extend(self.filter_flare(flares))
        elif self._cur_task == "bucket":
            observations.extend(self.filter_bucket(buckets))
        elif self._cur_task == "aruco":
            observations.extend(self.filter_flag(flags))
        else:
            return feature_msg  # Unknown task, return empty observations
        
        feature_msg.size = len(observations)

        feature_msg.observations = observations
        return feature_msg
      
    def _draw_boxes(self) -> np.ndarray:
        """Draw bounding boxes and labels on a BGR frame for debugging.

        Iterates detections (x, y, w, h, conf, cls) and draws filled label
        backgrounds plus confidence text to improve visibility.
        """
        frame = self._latest_raw_image
        if self.detections is None:
            return frame
        for det in self.detections:
            x = int(det["x"])
            y = int(det["y"])
            w = int(det["w"])
            h = int(det["h"])
            conf = float(det["conf"])
            cls_id = int(det["cls"])
            class_name = CLASS_NAMES.get(cls_id, "unknown")
            box_color = CLASS_COLORS.get(cls_id, (0,0,0))

            cv2.rectangle(frame, (x, y), (x + w, y + h), box_color, 4)

            label_text = f"{class_name}: {conf:.2f}"
            text_size = cv2.getTextSize(label_text, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 2)[0]
            label_y = max(0, y - 25)
            cv2.rectangle(frame, (x, label_y), (x + text_size[0] + 10, y), box_color, -1)
            cv2.putText(frame, label_text, (x + 5, max(0, y - 7)), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

        return frame

    def _filter_and_transform_boxes(
        self,
        predictions: np.ndarray,
        scale: float,
        pad_w: int,
        pad_h: int,
        original_shape: Sequence[int],
    ) -> List[dict]:
        """Filter raw model predictions and map them back to the original image.

        This function performs:
        - Confidence thresholding to remove low-confidence rows.
        - Handles percentage-formatted boxes (0..1) by scaling to model input.
        - Discards boxes with non-positive width/height.
        - Unpads and unscales boxes from model input space back to original image
          coordinates using (pad_w, pad_h) and scale values produced during
          preprocessing.
        - Clamps boxes to image bounds and returns a list of detection dicts.

        Args:
            predictions: Raw predictions array of shape (N, 6) with
                [x, y, w, h, confidence, class_id].
            scale: Scale factor used during preprocessing (model_size / original_size).
            pad_w: Horizontal padding applied during preprocessing.
            pad_h: Vertical padding applied during preprocessing.
            original_shape: Tuple of (original_height, original_width).
            conf_threshold: Optional confidence threshold override; uses instance
                default if not provided.

        Returns:
            List of detection dicts with keys: x, y, w, h, conf, cls.
        """

        self.detections = []
        original_h, original_w = original_shape
        input_w, input_h = 256, 256

        # Fast-return when there are no predictions
        if predictions is None or predictions.size == 0:
            return []

        predictions = np.asarray(predictions, dtype=np.float32)

        # 1) Confidence filtering
        confidences = predictions[:, 4]
        valid_mask = confidences >= self._conf_threshold
        if not np.any(valid_mask):
            return []

        # Keep only valid rows
        boxes = predictions[valid_mask, :4].copy()
        confidences = confidences[valid_mask]
        class_ids = predictions[valid_mask, 5].astype(np.int32)

        # 2) If coordinates are percentages (<=1.5 heuristic), scale to input dims
        percent_mask = (boxes[:, 2] <= 1.5) & (boxes[:, 3] <= 1.5)
        if np.any(percent_mask):
            boxes[percent_mask, 0] *= float(input_w)
            boxes[percent_mask, 1] *= float(input_h)
            boxes[percent_mask, 2] *= float(input_w)
            boxes[percent_mask, 3] *= float(input_h)

        # 3) Discard degenerate boxes
        widths = boxes[:, 2] - boxes[:, 0]
        heights = boxes[:, 3] - boxes[:, 1]
        valid_dims_mask = (widths > 0.0) & (heights > 0.0)
        if not np.any(valid_dims_mask):
            return []

        boxes = boxes[valid_dims_mask]
        confidences = confidences[valid_dims_mask]
        class_ids = class_ids[valid_dims_mask]
        widths = widths[valid_dims_mask]
        heights = heights[valid_dims_mask]

        # 4) Unpad and unscale to original image coordinates
        real_x = np.maximum(0, ((boxes[:, 0] - pad_w) / scale).astype(np.int32))
        real_y = np.maximum(0, ((boxes[:, 1] - pad_h) / scale).astype(np.int32))
        real_w = np.maximum(0, (widths / scale).astype(np.int32))
        real_h = np.maximum(0, (heights / scale).astype(np.int32))

        # 5) Clamp sizes to remain in-frame
        max_w = np.maximum(0, original_w - real_x)
        max_h = np.maximum(0, original_h - real_y)
        real_w = np.minimum(max_w, real_w)
        real_h = np.minimum(max_h, real_h)

        valid_size_mask = (real_w > 0) & (real_h > 0)
        if not np.any(valid_size_mask):
            return []

        boxes = boxes[valid_size_mask]
        confidences = confidences[valid_size_mask]
        class_ids = class_ids[valid_size_mask]
        real_x = real_x[valid_size_mask]
        real_y = real_y[valid_size_mask]
        real_w = real_w[valid_size_mask]
        real_h = real_h[valid_size_mask]

        # Build detection list
        for index in range(real_x.shape[0]):
            self.detections.append(
                {
                    "x": float(real_x[index]),
                    "y": float(real_y[index]),
                    "w": float(real_w[index]),
                    "h": float(real_h[index]),
                    "conf": float(confidences[index]),
                    "cls": float(class_ids[index])
                }
            )

        return self.detections