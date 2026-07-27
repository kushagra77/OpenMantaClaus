from typing import Optional

import cv2
import numpy as np

from .cv_node_params import CVNodeParams


class CameraControl:
    def __init__(self, cap: cv2.VideoCapture, params: CVNodeParams) -> None:
        self._cap = cap
        self._params = params

        self._img_size = (params.processing_width, params.processing_height)
        self._camera_matrix = params.camera_matrix
        self._dist_coeffs = params.dist_coeffs

        self._enable_auto_exposure = params.enable_auto_exposure
        self._manual_override_exposure = params.manual_override_exposure
        self._min_exposure = params.min_exposure
        self._max_exposure = params.max_exposure
        self._brightness = params.brightness
        self._target_brightness = params.target_brightness
        self._exposure_check_interval = max(1, params.exposure_check_interval)
        self._current_exposure = self._min_exposure
        self._exposure_frame_counter = 0

        self._map1: Optional[np.ndarray] = None
        self._map2: Optional[np.ndarray] = None

    def setup_camera(self) -> None:
        if not self._cap.isOpened():
            return

        fourcc = self._params.capture_fourcc
        fourcc = (fourcc + "    ")[:4]

        self._cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        self._cap.set(cv2.CAP_PROP_AUTOFOCUS, self._params.capture_autofocus)
        self._cap.set(cv2.CAP_PROP_AUTO_WB, self._params.capture_auto_white_balance)
        self._cap.set(cv2.CAP_PROP_FOCUS, self._params.capture_focus)
        self._cap.set(
            cv2.CAP_PROP_WHITE_BALANCE_BLUE_U,
            self._params.capture_white_balance_blue_u,
        )

        self._cap.set(cv2.CAP_PROP_GAIN, 0)
        self._cap.set(cv2.CAP_PROP_BRIGHTNESS, self._brightness)
        self._configure_exposure_mode()
        self._cap.set(cv2.CAP_PROP_BACKLIGHT, self._params.capture_backlight)
        self._cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*fourcc))
        self._cap.set(cv2.CAP_PROP_FRAME_WIDTH, self._params.capture_frame_width)
        self._cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self._params.capture_frame_height)
        self._cap.set(cv2.CAP_PROP_FPS, self._params.capture_fps)

        self.new_camera_matrix, _ = cv2.getOptimalNewCameraMatrix(
            self._camera_matrix,
            self._dist_coeffs,
            self._img_size,
            0.0,
            self._img_size,
        )
        self._map1, self._map2 = cv2.initUndistortRectifyMap(
            self._camera_matrix,
            self._dist_coeffs,
            None,
            self.new_camera_matrix,
            self._img_size,
            cv2.CV_32FC1,
        )

    def process_frame(self, frame: np.ndarray) -> np.ndarray:
        if frame.shape[1] != self._img_size[0] or frame.shape[0] != self._img_size[1]:
            frame = cv2.resize(frame, self._img_size, interpolation=cv2.INTER_AREA)
        if self._map1 is not None and self._map2 is not None:
            frame = cv2.remap(frame, self._map1, self._map2, cv2.INTER_LINEAR)

        self._exposure_frame_counter += 1
        if self._exposure_frame_counter % self._exposure_check_interval == 0:
            if not (self._manual_override_exposure > 0 or self._enable_auto_exposure == 1):
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                p90 = np.percentile(gray, 90)
                p95 = np.percentile(gray, 95)

                highlight_pixels = gray[(gray >= p90) & (gray <= p95)]
                highlight_brightness = float(np.mean(highlight_pixels)) if len(highlight_pixels) > 0 else float(p95)

                error = self._target_brightness - highlight_brightness

                if abs(error) > 10:
                    step = int(error * 0.9)
                    self._current_exposure += step
                    self._current_exposure = max(self._min_exposure, min(self._max_exposure, self._current_exposure))
                    self._cap.set(cv2.CAP_PROP_EXPOSURE, self._current_exposure)

        return frame

    def _configure_exposure_mode(self) -> None:
        # V4L2/OpenCV convention: 3.0 means auto exposure, 1.0 means manual exposure.
        if self._manual_override_exposure > 0:
            self._current_exposure = self._manual_override_exposure
            self._cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1.0)
            self._cap.set(cv2.CAP_PROP_EXPOSURE, self._current_exposure)
            return

        if self._enable_auto_exposure == 1:
            self._cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 3.0)
            return

        self._current_exposure = max(self._min_exposure, min(self._max_exposure, self._current_exposure))
        self._cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1.0)
        self._cap.set(cv2.CAP_PROP_EXPOSURE, self._current_exposure)
