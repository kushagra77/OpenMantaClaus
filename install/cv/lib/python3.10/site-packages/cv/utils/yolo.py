import os
from pathlib import Path
from typing import Optional, Tuple
import numpy as np
from ai_edge_litert.interpreter import Interpreter

os.environ["OPENCV_FFMPEG_READ_ATTEMPTS"] = "10000"


def _resolve_model_path(model_path: Optional[str | Path] = None) -> Path:
    """Resolve a model path. Prefer an explicit `model_path` argument, then
    `CV_YOLO_MODEL_PATH` environment variable. Do not fall back to any
    hardcoded repository paths — caller must provide a valid path.
    """
    if model_path is not None:
        candidate = Path(model_path)
        if candidate.exists():
            return candidate
        raise FileNotFoundError(f"YOLO model not found: {candidate}")

    env_model_path = os.environ.get("CV_YOLO_MODEL_PATH")
    if env_model_path:
        candidate = Path(env_model_path)
        if candidate.exists():
            return candidate

    raise FileNotFoundError("Could not resolve YOLO model path: provide `model_path` parameter or set CV_YOLO_MODEL_PATH")


class Yolo:
    def __init__(self, model_path: Optional[str | Path] = None, num_threads: int = 2) -> None:
        self._model_path = _resolve_model_path(model_path)
        
        # Initialize the interpreter
        self._interpreter = Interpreter(model_path=str(self._model_path), num_threads=num_threads)
        self._interpreter.allocate_tensors()

        # Cache tensor details for fast internal lookup
        self._input_details = self._interpreter.get_input_details()[0]
        self._output_details = self._interpreter.get_output_details()[0]
        
        self._input_index = self._input_details["index"]
        self._output_index = self._output_details["index"]

    def get_details(self) -> Tuple[dict, dict]:
        """Expose details so the CV Node can cache quantization constants."""
        return self._input_details, self._output_details

    def infer(self, input_data: np.ndarray) -> np.ndarray:
        """Strictly perform tensor assignment and model invocation."""
        self._interpreter.set_tensor(self._input_index, input_data)
        self._interpreter.invoke()
        return self._interpreter.get_tensor(self._output_index)