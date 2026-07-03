import cv2
import numpy as np
import time, os
from pathlib import Path
from ai_edge_litert.interpreter import Interpreter, load_delegate
os.environ["OPENCV_FFMPEG_READ_ATTEMPTS"] = "10000"
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image as ROSImage
from cv_bridge import CvBridge
from rclpy.qos import QoSProfile, QoSReliabilityPolicy
from rclpy.qos import qos_profile_sensor_data # Import the sensor profile

# --- Configuration ---
MODEL_PATH = [
    "scripts/yolo/runs/detect/pretrain_generalist_v2/weights/best_saved_model/best_int8.tflite",
    "scripts/yolo/runs/detect/finetune_specialist_v1/weights/best_saved_model/best_int8.tflite",
    
    
]
INPUT_PATH = "/camera/img"

CONF_THRESHOLD = 0.8 

IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}
VIDEO_EXTENSIONS = {".mp4", ".avi", ".mov", ".mkv", ".m4v", ".webm"}

# Class names and colors
CLASS_NAMES = {
    0: "flag",
    1: "gate",
    2: "flare",
    3: "bucket"
}

CLASS_COLORS = {
    0: (255, 0, 0),      # flag - Blue
    1: (0, 255, 0),      # gate - Green
    2: (0, 165, 255),    # flare - Orange
    3: (255, 0, 255)     # bucket - Magenta
}

# Color constants and thresholds copied from cv/utils/filter_detections.py + params.yaml
RED = 1
BLUE = 2
YELLOW = 3
UNKNOWN = 0

RED_LOWER = [0, 0, 80]
RED_HIGHER = [140, 160, 255]
BLUE_LOWER = [50, 150, 0]
BLUE_HIGHER = [160, 255, 135]
YELLOW_LOWER = [150, 100, 110]
YELLOW_HIGHER = [190, 150, 200]
ERODE_KERNEL_SIZE = 3
FLARE_PERCENTAGE_THRESHOLD = 0.2
BUCKET_PERCENTAGE_THRESHOLD = 0.7

ERODE_KERNEL = cv2.getStructuringElement(cv2.MORPH_RECT, (ERODE_KERNEL_SIZE, ERODE_KERNEL_SIZE))
COLOR_NAME = {
    UNKNOWN: "UNKNOWN",
    RED: "RED",
    BLUE: "BLUE",
    YELLOW: "YELLOW",
}


def draw_gate_dividers(frame, x, y, w, h, color, parts=20):
    if w <= 0 or h <= 0 or parts <= 1:
        return

    for index in range(1, parts):
        x_pos = int(round(x + (w * index / parts)))
        cv2.line(frame, (x_pos, y), (x_pos, y + h), color, 1)

def get_mask(frame, color):
    if color == RED:
        lower = np.array(RED_LOWER)
        upper = np.array(RED_HIGHER)
    elif color == BLUE:
        lower = np.array(BLUE_LOWER)
        upper = np.array(BLUE_HIGHER)
    elif color == YELLOW:
        lower = np.array(YELLOW_LOWER)
        upper = np.array(YELLOW_HIGHER)
    else:
        raise ValueError(f"Unknown color: {color}")

    yuv = cv2.cvtColor(frame, cv2.COLOR_BGR2YUV)
    mask = cv2.inRange(yuv, lower, upper)
    mask = cv2.erode(mask, ERODE_KERNEL, iterations=1)
    return mask

def get_flare_color(frame):
    if frame is None or frame.size == 0:
        return UNKNOWN

    red_mask = get_mask(frame, RED)
    yellow_mask = get_mask(frame, YELLOW)

    red_count = cv2.countNonZero(red_mask)
    yellow_count = cv2.countNonZero(yellow_mask)
    all_count = frame.shape[0] * frame.shape[1]
    if all_count <= 0:
        return UNKNOWN

    final_color = UNKNOWN
    if (red_count / all_count) > FLARE_PERCENTAGE_THRESHOLD:
        if red_count > yellow_count * 1.5:
            final_color = RED

    if (yellow_count / all_count) > FLARE_PERCENTAGE_THRESHOLD:
        if yellow_count > red_count * 1.5:
            final_color = YELLOW

    return final_color

def get_bucket_color(frame):
    if frame is None or frame.size == 0:
        return UNKNOWN

    red_mask = get_mask(frame, RED)
    blue_mask = get_mask(frame, BLUE)

    red_count = cv2.countNonZero(red_mask)
    blue_count = cv2.countNonZero(blue_mask)
    all_count = frame.shape[0] * frame.shape[1]
    if all_count <= 0:
        return UNKNOWN

    final_color = UNKNOWN

    if (red_count / all_count) > BUCKET_PERCENTAGE_THRESHOLD:
        if red_count > blue_count * 2.0:
            final_color = RED

    if (blue_count / all_count) > BUCKET_PERCENTAGE_THRESHOLD:
        if blue_count > red_count * 2.0:
            final_color = BLUE

    return final_color

def draw_detection_labels(frame, real_x, real_y, real_w, real_h, box_color, top_text, bottom_text):
    text_scale = 0.6
    text_thickness = 2
    text_color = (255, 255, 255)
    pad = 10
    box_h = 25
    frame_h, frame_w = frame.shape[:2]

    top_size = cv2.getTextSize(top_text, cv2.FONT_HERSHEY_SIMPLEX, text_scale, text_thickness)[0]
    top_x2 = min(frame_w - 1, real_x + top_size[0] + pad)
    top_y1 = max(0, real_y - box_h)
    top_y2 = max(0, min(frame_h - 1, real_y))
    cv2.rectangle(frame, (real_x, top_y1), (top_x2, top_y2), box_color, -1)
    top_text_y = max(15, min(frame_h - 5, top_y2 - 7))
    cv2.putText(frame, top_text, (real_x + 5, top_text_y), cv2.FONT_HERSHEY_SIMPLEX, text_scale, text_color, text_thickness)

    bottom_size = cv2.getTextSize(bottom_text, cv2.FONT_HERSHEY_SIMPLEX, text_scale, text_thickness)[0]
    bottom_x2 = min(frame_w - 1, real_x + bottom_size[0] + pad)
    bottom_y1 = max(0, min(frame_h - 1, real_y + real_h))
    bottom_y2 = max(0, min(frame_h - 1, bottom_y1 + box_h))
    cv2.rectangle(frame, (real_x, bottom_y1), (bottom_x2, bottom_y2), box_color, -1)
    bottom_text_y = max(15, min(frame_h - 5, bottom_y2 - 7))
    cv2.putText(frame, bottom_text, (real_x + 5, bottom_text_y), cv2.FONT_HERSHEY_SIMPLEX, text_scale, text_color, text_thickness)

def draw_detection(frame, det, original_w, original_h, scale, pad_w, pad_h):
    x, y, w, h = det["x"], det["y"], det["w"], det["h"]
    conf = det["conf"]
    cls_id = det["cls"]

    real_x = int((x - pad_w) / scale)
    real_y = int((y - pad_h) / scale)
    real_w = int(w / scale)
    real_h = int(h / scale)

    real_x = max(0, real_x)
    real_y = max(0, real_y)
    real_w = min(original_w - real_x, real_w)
    real_h = min(original_h - real_y, real_h)

    if real_w <= 0 or real_h <= 0:
        return

    class_name = CLASS_NAMES.get(cls_id, f"Unknown {int(cls_id)}")
    box_color = CLASS_COLORS.get(cls_id, (255, 255, 255))

    cv2.rectangle(frame, (real_x, real_y), (real_x + real_w, real_y + real_h), box_color, 4)
    if cls_id == 1:
        draw_gate_dividers(frame, real_x, real_y, real_w, real_h, box_color)

    color_suffix = ""
    if cls_id in (2, 3):
        crop = frame[real_y:real_y + real_h, real_x:real_x + real_w]
        color_id = get_flare_color(crop) if cls_id == 2 else get_bucket_color(crop)
        color_suffix = f" | {COLOR_NAME.get(color_id, 'UNKNOWN')}"

    top_text = f"{class_name}: {conf:.2f}{color_suffix}"
    bottom_text = f"{class_name}{color_suffix}"
    draw_detection_labels(frame, real_x, real_y, real_w, real_h, box_color, top_text, bottom_text)

def letterbox_image(image, expected_size=(256, 256), color=(114, 114, 114)):
    ih, iw = image.shape[:2]
    ew, eh = expected_size
    scale = min(ew / iw, eh / ih)
    nw = int(iw * scale)
    nh = int(ih * scale)
    
    image_resized = cv2.resize(image, (nw, nh), interpolation=cv2.INTER_LINEAR)
    padded_image = np.full((eh, ew, 3), color, dtype=np.uint8)
    
    pad_w = (ew - nw) // 2
    pad_h = (eh - nh) // 2
    padded_image[pad_h:pad_h+nh, pad_w:pad_w+nw] = image_resized
    
    return padded_image, scale, pad_w, pad_h

def count_video_frames(video_path):
    cap = cv2.VideoCapture(str(video_path))
    if not cap.isOpened():
        return 0

    frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    if frame_count > 0:
        cap.release()
        return frame_count

    frame_count = 0
    while True:
        ret, _ = cap.read()
        if not ret:
            break
        frame_count += 1
    cap.release()
    return frame_count

def resolve_input_source(input_path_str):
    source_path = Path(input_path_str)

    # If input ends with a slash treat as a directory (must exist)
    if input_path_str.endswith(("/", "\\")):
        if not source_path.exists() or not source_path.is_dir():
            raise ValueError("Input ends with '/' but is not a directory.")
        image_paths = sorted(
            [
                p
                for p in source_path.iterdir()
                if p.is_file() and p.suffix.lower() in IMAGE_EXTENSIONS
            ]
        )
        if not image_paths:
            raise ValueError("Error: No images found.")
        return "images", image_paths, len(image_paths)

    # If it's an existing filesystem path, preserve image/video logic
    if source_path.exists():
        suffix = source_path.suffix.lower()
        if suffix in IMAGE_EXTENSIONS:
            repeated = [source_path] * 30
            return "images", repeated, len(repeated)

        if suffix in VIDEO_EXTENSIONS:
            frame_count = count_video_frames(source_path)
            if frame_count <= 0:
                raise ValueError("Error: Could not read video frames.")
            return "video", source_path, frame_count

        raise FileNotFoundError(f"Input path not supported or not found: {source_path}")

    # If path doesn't exist on disk and doesn't end with '/', treat as ROS topic name
    # Also ensure it doesn't look like an image/video extension
    suffix = source_path.suffix.lower()
    if suffix in IMAGE_EXTENSIONS or suffix in VIDEO_EXTENSIONS:
        raise FileNotFoundError(f"Input path not found: {source_path}")

    # Treat as ROS topic
    return "ros_topic", input_path_str, None


def load_model_session(model_path):
    interpreter = Interpreter(model_path=model_path, num_threads=1)
    interpreter.allocate_tensors()

    input_details = interpreter.get_input_details()[0]
    output_details = interpreter.get_output_details()[0]
    input_scale, input_zero_point = input_details["quantization"]
    out_scale, out_zero_point = output_details["quantization"]

    return {
        "model_path": model_path,
        "interpreter": interpreter,
        "input_details": input_details,
        "output_details": output_details,
        "input_scale": input_scale,
        "input_zero_point": input_zero_point,
        "out_scale": out_scale,
        "out_zero_point": out_zero_point,
        "total_inference_ms": 0.0,
        "idx": 0,
    }


def create_model_states(model_paths):
    model_states = []
    for index, model_path in enumerate(model_paths):
        print(f"Loading Edge TPU / CPU Interpreter with model {index}: {model_path}...")
        model_state = load_model_session(model_path)
        model_state["index"] = index
        model_state["window_name"] = f"Model {index}"
        model_states.append(model_state)
    return model_states


def run_model_inference(frame, model_state):
    original_h, original_w = frame.shape[:2]

    inference_start = time.perf_counter()
    img_padded, scale, pad_w, pad_h = letterbox_image(frame, (256, 256))
    img_rgb = cv2.cvtColor(img_padded, cv2.COLOR_BGR2RGB)
    img_normalized = img_rgb.astype(np.float32) / 255.0

    input_scale = model_state["input_scale"]
    input_zero_point = model_state["input_zero_point"]
    input_details = model_state["input_details"]

    if input_scale > 0:
        if input_details["dtype"] == np.int8:
            img_input = np.clip(
                np.round(img_normalized / input_scale + input_zero_point), -128, 127
            ).astype(np.int8)
        else:
            img_input = np.clip(
                np.round(img_normalized / input_scale + input_zero_point), 0, 255
            ).astype(np.uint8)
    else:
        img_input = img_normalized

    input_data = np.expand_dims(img_input, axis=0)

    interpreter = model_state["interpreter"]
    output_details = model_state["output_details"]
    out_scale = model_state["out_scale"]
    out_zero_point = model_state["out_zero_point"]

    interpreter.set_tensor(input_details["index"], input_data)
    interpreter.invoke()

    output_data = interpreter.get_tensor(output_details["index"])
    if out_scale > 0:
        output_data = (output_data.astype(np.float32) - out_zero_point) * out_scale

    predictions = output_data[0]
    best_pred = max(predictions, key=lambda x: x[4])

    valid_detections = []
    valid_mask = predictions[:, 4] > CONF_THRESHOLD
    valid_rows = predictions[valid_mask]

    for row in valid_rows:
        x1, y1, x2, y2, confidence, cls_id = row[0], row[1], row[2], row[3], row[4], row[5]
        if x2 <= 1.5 and y2 <= 1.5:
            x1 *= 256.0
            y1 *= 256.0
            x2 *= 256.0
            y2 *= 256.0

        valid_detections.append({
            "x": int(x1),
            "y": int(y1),
            "w": int(x2 - x1),
            "h": int(y2 - y1),
            "conf": float(confidence),
            "cls": int(cls_id),
        })

    inference_ms = (time.perf_counter() - inference_start) * 1000.0
    model_state["total_inference_ms"] += inference_ms
    model_state["idx"] += 1
    avg_inference_ms = model_state["total_inference_ms"] / max(1, model_state["idx"])

    return {
        "frame": frame,
        "original_w": original_w,
        "original_h": original_h,
        "scale": scale,
        "pad_w": pad_w,
        "pad_h": pad_h,
        "best_pred": best_pred,
        "valid_detections": valid_detections,
        "inference_ms": inference_ms,
        "avg_inference_ms": avg_inference_ms,
        "frame_index": model_state["idx"],
    }


def draw_results(frame, inference_result):
    for det in inference_result["valid_detections"]:
        draw_detection(
            frame,
            det,
            inference_result["original_w"],
            inference_result["original_h"],
            inference_result["scale"],
            inference_result["pad_w"],
            inference_result["pad_h"],
        )


def display_model_windows(model_states, frame, frame_label, total_items):
    for model_state in model_states:
        inference_result = run_model_inference(frame.copy(), model_state)
        print(
            f"\n[model {model_state['index']}][{frame_label}/{total_items}] "
            f"Max Conf: {inference_result['best_pred'][4]:.3f} | "
            f"Raw Box: x1={inference_result['best_pred'][0]:.2f}, "
            f"y1={inference_result['best_pred'][1]:.2f}, "
            f"x2={inference_result['best_pred'][2]:.2f}, "
            f"y2={inference_result['best_pred'][3]:.2f}"
        )
        print(
            f"Inference time: {inference_result['inference_ms']:.2f} ms | "
            f"Avg: {inference_result['avg_inference_ms']:.2f} ms"
        )
        print(f"Boxes passing threshold (Ready to draw): {len(inference_result['valid_detections'])}")

        draw_results(inference_result["frame"], inference_result)
        cv2.imshow(model_state["window_name"], inference_result["frame"])

    return cv2.waitKey(0) & 0xFF

def main():
    if not isinstance(MODEL_PATH, (list, tuple)):
        raise TypeError("MODEL_PATH must be a list or tuple of paths.")

    model_states = create_model_states(MODEL_PATH)
    if not model_states:
        print("No model paths configured.")
        return

    try:
        source_type, source_data, total_items = resolve_input_source(INPUT_PATH)
    except (FileNotFoundError, ValueError) as e:
        print(str(e))
        return

    for model_state in model_states:
        cv2.namedWindow(model_state["window_name"], cv2.WINDOW_NORMAL)

    if source_type == "video":
        cap = cv2.VideoCapture(str(source_data))
        if not cap.isOpened():
            print("Error: Could not open video.")
            return
        frame_iter = range(1, total_items + 1)
    else:
        cap = None
        frame_iter = enumerate(source_data, start=1)

    # ROS topic handling: subscribe to topic publishing sensor_msgs/Image
    if source_type == "ros_topic":
        topic_name = source_data

        class ROSInferenceNode(Node):
            def __init__(self, node_name, model_states):
                super().__init__(node_name)
                self.bridge = CvBridge()
                self.model_states = model_states

                qos = QoSProfile(depth=10)
                self.sub = self.create_subscription(ROSImage, topic_name, self.callback, qos_profile_sensor_data)
                out_topic = topic_name.rstrip('/') + '_inference'
                self.pub = self.create_publisher(ROSImage, out_topic, qos_profile_sensor_data)
                self.get_logger().info(f"Subscribed to {topic_name}, publishing to {out_topic}")

            def callback(self, msg):
                try:
                    frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
                except Exception as e:
                    self.get_logger().error(f"Failed to convert image msg: {e}")
                    return

                publish_frame = None

                for model_state in self.model_states:
                    inference_result = run_model_inference(frame.copy(), model_state)
                    self.get_logger().info(
                        f"[model {model_state['index']}][{inference_result['frame_index']}] "
                        f"Inference {inference_result['inference_ms']:.2f} ms | "
                        f"Avg: {inference_result['avg_inference_ms']:.2f} ms | "
                        f"Boxes: {len(inference_result['valid_detections'])}"
                    )

                    draw_results(inference_result["frame"], inference_result)
                    cv2.imshow(model_state["window_name"], inference_result["frame"])

                    if model_state["index"] == 0:
                        publish_frame = inference_result["frame"]

                cv2.waitKey(1)

                if publish_frame is not None:
                    try:
                        out_msg = self.bridge.cv2_to_imgmsg(publish_frame, encoding='bgr8')
                        out_msg.header = msg.header
                        self.pub.publish(out_msg)
                    except Exception as e:
                        self.get_logger().error(f"Failed to publish annotated image: {e}")

        # Initialize rclpy and run the node
        rclpy.init()
        node = ROSInferenceNode('yolo_inference_node', model_states)
        try:
            rclpy.spin(node)
        except KeyboardInterrupt:
            node.get_logger().info('Shutting down ROS inference node.')
        finally:
            node.destroy_node()
            rclpy.shutdown()
        return

    for frame_token in frame_iter:
        if source_type == "video":
            idx = frame_token
            ret, frame = cap.read()
            if not ret:
                break
            # resize to 360 p
            frame = cv2.resize(frame, (640, 360), interpolation=cv2.INTER_AREA)
        else:
            idx, image_path = frame_token
            frame = cv2.imread(str(image_path))
            if frame is None:
                continue

        key = display_model_windows(model_states, frame, idx, total_items)
        if key == ord('q'):
            if cap is not None:
                cap.release()
            cv2.destroyAllWindows()
            return

    if cap is not None:
        cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()