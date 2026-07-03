#!/usr/bin/env python3
"""
Chessboard calibration script for camera intrinsic calibration.
Reads images from bags/chess/, detects chessboard patterns, and performs camera calibration.
"""

import cv2
import numpy as np
import os
import pickle
from pathlib import Path

class ChessboardCalibrator:
    def __init__(self, image_dir, chessboard_size=(14, 9), cell_size_cm=3.0):
        """
        Initialize the chessboard calibrator.
        
        Args:
            image_dir: Directory containing the calibration images
            chessboard_size: Tuple of (width, height) in cells. Default: (14, 9)
            cell_size_cm: Size of each cell in centimeters. Default: 3.0 cm
        """
        self.image_dir = image_dir
        self.chessboard_size = chessboard_size
        self.cell_size_m = cell_size_cm / 100.0  # Convert to meters
        
        # Termination criteria for corner refinement
        self.criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
        
        # Prepare object points (0,0,0), (1,0,0), (2,0,0)..., (13,8,0)
        self.objp = np.zeros((chessboard_size[0] * chessboard_size[1], 3), np.float32)
        self.objp[:, :2] = np.mgrid[0:chessboard_size[0], 0:chessboard_size[1]].T.reshape(-1, 2)
        # Scale by cell size (in meters)
        self.objp *= self.cell_size_m
        
        self.objpoints = []  # 3D points in real world space
        self.imgpoints = []  # 2D points in image plane
        self.image_size = None
        self.valid_images = []
        
    def load_images(self):
        """Load all valid image files from the directory."""
        images = []
        for filename in sorted(os.listdir(self.image_dir)):
            if filename.lower().endswith(('.jpg', '.png', '.jpeg')):
                images.append(os.path.join(self.image_dir, filename))
        images.sort(key = lambda x: int(x[len(self.image_dir) + len("img_"):-len(".jpg")]))
        return images[::3]
    def is_detection_valid(self, corners):
      """
      Hyper-selective validation of the chessboard grid.
      Verifies monotonicity, collinearity, orthogonality, and distance consistency.
      """
      if corners is None or len(corners) != (self.chessboard_size[0] * self.chessboard_size[1]):
          return False

      # Reshape to (Rows, Cols, 2) -> (8, 13, 2)
      pts = corners.reshape(self.chessboard_size[1], self.chessboard_size[0], 2)
      origin = pts[0, 0]
      
      # 1. Global Distance & Monotonicity Check
      # We expect the distance from the first point to increase as we move along rows/cols
      for r in range(self.chessboard_size[1]):
          row_pts = pts[r, :, :]
          
          # X-Monotonicity: Ensure X is strictly increasing or decreasing
          x_diffs = np.diff(row_pts[:, 0])
          if not (np.all(x_diffs > 1.0) or np.all(x_diffs < -1.0)):
              return False

          for c in range(self.chessboard_size[0]):
              # Verify distance growth from the origin (0,0)
              # In a valid grid, distance should generally increase as indices r and c increase
              dist_from_origin = np.linalg.norm(pts[r, c] - origin)
              
              # Distance consistency check: 
              # If we are at index (r, c), the distance should be roughly proportional 
              # to sqrt(r^2 + c^2). We check that it isn't smaller than previous points.
              if c > 0:
                  if dist_from_origin <= np.linalg.norm(pts[r, c-1] - origin):
                      return False # Grid is 'folding' back toward origin

      # 2. Local Collinearity Check (Rows)
      for r in range(self.chessboard_size[1]):
          row_pts = pts[r, :, :]
          for c in range(2, self.chessboard_size[0]):
              v1 = row_pts[c-1] - row_pts[c-2]
              v2 = row_pts[c] - row_pts[c-1]
              # Cross product of normalized vectors checks for straight lines
              cross_prod = np.abs(np.cross(v1/np.linalg.norm(v1), v2/np.linalg.norm(v2)))
              if cross_prod > 0.1: # Max ~5.7 degrees deviation
                  return False

      # 3. Orthogonality Check (~70 to 110 degrees)
      # Verifies the 90-degree relationship between rows and columns
      for r in range(1, self.chessboard_size[1]):
          for c in range(1, self.chessboard_size[0]):
              v_row = pts[r, c-1] - pts[r, c]
              v_col = pts[r-1, c] - pts[r, c]
              
              cosine_angle = np.dot(v_row, v_col) / (np.linalg.norm(v_row) * np.linalg.norm(v_col))
              angle = np.degrees(np.arccos(np.clip(cosine_angle, -1.0, 1.0)))
              
              if not (70 < angle < 110): # Allows for perspective skew
                  return False

      # 4. Global Convexity Check
      hull = cv2.convexHull(corners.reshape(-1, 2))
      if not cv2.isContourConvex(hull):
          return False

      return True
    def detect_chessboard(self, image_path):
        """
        Detect chessboard corners in a single image.
        
        Args:
            image_path: Path to the image file
            
        Returns:
            corners: Detected corner points if successful, None otherwise
            gray: Grayscale image
        """
        img = cv2.imread(image_path)
        if img is None:
            print(f"Failed to load image: {image_path}")
            return None, None
        
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        
        # Find chessboard corners
        # ret, corners = cv2.findChessboardCorners(gray, self.chessboard_size, None)
        ret, corners = cv2.findChessboardCornersSB(img, self.chessboard_size, cv2.CALIB_CB_ACCURACY + cv2.CALIB_CB_EXHAUSTIVE + cv2.CALIB_CB_NORMALIZE_IMAGE + cv2.CALIB_CB_FILTER_QUADS)
        
        if ret and self.is_detection_valid(corners):
            # Refine corner positions
            corners2 = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), self.criteria)
            return corners2, gray
        else:
            return None, gray
    
    def calibrate(self):
        """
        Perform camera calibration using detected chessboard patterns.
        
        Returns:
            ret: Calibration success flag
            mtx: Camera intrinsic matrix
            dist: Distortion coefficients
            rvecs: Rotation vectors
            tvecs: Translation vectors
        """
        images = self.load_images()
        print(f"Found {len(images)} image files in {self.image_dir}")
        
        if not images:
            print("No images found in the directory.")
            return False, None, None, None, None
        
        print("\nDetecting chessboard patterns...")
        print("Press any key to continue to the next image, or 'q' to quit.\n")
        
        for i, image_path in enumerate(images):
            print(f"Processing {i+1}/{len(images)}: {os.path.basename(image_path)}", end=" ")
            
            img = cv2.imread(image_path)
            corners, gray = self.detect_chessboard(image_path)
            
            # Display the image
            display_img = img.copy() if img is not None else gray
            
            if corners is not None:
                self.objpoints.append(self.objp)
                self.imgpoints.append(corners)
                self.valid_images.append(image_path)
                self.image_size = gray.shape[::-1]
                print("✓ Chessboard detected")
                # Draw detected corners on the image
                cv2.drawChessboardCorners(display_img, self.chessboard_size, corners, True)
            else:
                print("✗ No chessboard found")
            
            # Show the image
            cv2.imshow(f'Chessboard Detection', display_img)
            key = cv2.waitKey(10)  # Wait for user input
            if key == ord('q'):  # Quit if 'q' is pressed
                print("\nQuitting early...")
                cv2.destroyAllWindows()
                break
        
        cv2.destroyAllWindows()
        
        
        print(f"\nSuccessfully detected chessboard in {len(self.objpoints)}/{len(images)} images")
        
        if len(self.objpoints) < 3:
            print("ERROR: Need at least 3 valid images for calibration.")
            return False, None, None, None, None
        
        print("\nPerforming camera calibration...")
        ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(
            self.objpoints, self.imgpoints, self.image_size, None, None
        )
        
        if ret:
            print("✓ Calibration successful!")
            print(f"\nCalibration Results (Initial):")
            print(f"Camera Matrix:\n{mtx}")
            print(f"\nDistortion Coefficients:\n{dist}")
            print(f"Overall Reprojection Error: {ret:.4f} pixels")
            
            # Calculate per-image reprojection errors
            print("\n" + "="*60)
            print("Analyzing per-image reprojection errors...")
            print("="*60)
            return self.remove_high_error_images(mtx, dist, rvecs, tvecs)
        else:
            print("✗ Calibration failed.")
        
        return ret, mtx, dist, rvecs, tvecs
    
    def remove_high_error_images(self, mtx, dist, rvecs, tvecs, threshold_percentile=70):
        """
        Calculate per-image reprojection errors, identify outliers, and recalibrate.
        
        Args:
            mtx: Camera matrix from initial calibration
            dist: Distortion coefficients from initial calibration
            rvecs: Rotation vectors from initial calibration
            tvecs: Translation vectors from initial calibration
            threshold_percentile: Images above this percentile are considered outliers (default: 75th)
        """
        per_image_errors = []
        
        print(f"\nCalculating per-image reprojection errors:")
        print(f"{'Image':<50} {'Error (px)':>12}")
        print("-" * 65)
        
        for i, (objpts, imgpts, rvec, tvec) in enumerate(zip(
            self.objpoints, self.imgpoints, rvecs, tvecs
        )):
            # Project object points to image plane
            projected_points, _ = cv2.projectPoints(objpts, rvec, tvec, mtx, dist)
            projected_points = projected_points.reshape(-1, 2)
            
            # Calculate RMS error for this image
            error = np.sqrt(np.mean((imgpts.reshape(-1, 2) - projected_points)**2))
            per_image_errors.append(error)
            
            image_name = os.path.basename(self.valid_images[i])
            print(f"{image_name:<50} {error:>12.4f}")
        
        # Find threshold and outliers
        errors_array = np.array(per_image_errors)
        threshold = np.percentile(errors_array, threshold_percentile)
        outlier_indices = np.where(errors_array > threshold)[0]
        
        print("-" * 65)
        print(f"Threshold (75th percentile): {threshold:.4f} pixels")
        print(f"Mean error: {np.mean(errors_array):.4f} pixels")
        print(f"Std dev: {np.std(errors_array):.4f} pixels")
        print(f"Max error: {np.max(errors_array):.4f} pixels")
        print(f"\nFound {len(outlier_indices)} outlier images to remove:")
        
        if len(outlier_indices) > 0:
            for idx in outlier_indices:
                print(f"  - {os.path.basename(self.valid_images[idx])} (error: {per_image_errors[idx]:.4f} px)")
            
            # Remove outlier images
            self.objpoints = [self.objpoints[i] for i in range(len(self.objpoints)) if i not in outlier_indices]
            self.imgpoints = [self.imgpoints[i] for i in range(len(self.imgpoints)) if i not in outlier_indices]
            self.valid_images = [self.valid_images[i] for i in range(len(self.valid_images)) if i not in outlier_indices]
            
            print(f"\nRecalibrating with {len(self.objpoints)} images...")
            ret, mtx_new, dist_new, rvecs_new, tvecs_new = cv2.calibrateCamera(
                self.objpoints, self.imgpoints, self.image_size, None, None
            )
            
            if ret:
                print("✓ Recalibration successful!")
                print(f"\nRecalibration Results (After removing outliers):")
                print(f"Camera Matrix:\n{mtx_new}")
                print(f"\nDistortion Coefficients:\n{dist_new}")
                print(f"Overall Reprojection Error: {ret:.4f} pixels")
                
                # Compare improvements
                print(f"\nImprovement:")
                print(f"  Images used: {len(self.valid_images)} (removed {len(outlier_indices)})")
                # self.save_calibration(mtx_new, dist_new)
                return ret, mtx_new, dist_new, rvecs_new, tvecs_new
            else:
                print("✗ Recalibration failed.")
        else:
            print("No outlier images found.")
    
    def save_calibration(self, mtx, dist, output_file='camera_calibration.pkl'):
        """
        Save calibration results to a pickle file.
        
        Args:
            mtx: Camera intrinsic matrix
            dist: Distortion coefficients
            output_file: Output file name
        """
        calibration_data = {
            'camera_matrix': mtx,
            'distortion_coefficients': dist,
            'image_size': self.image_size,
            'valid_images': self.valid_images,
            'chessboard_size': self.chessboard_size,
            'cell_size_m': self.cell_size_m
        }
        
        with open(output_file, 'wb') as f:
            pickle.dump(calibration_data, f)
        
        print(f"\nCalibration data saved to {output_file}")
    
    def visualize_results(self, num_images=5):
        """
        Visualize chessboard detection on sample images.
        
        Args:
            num_images: Number of sample images to visualize
        """
        if not self.valid_images:
            print("No valid images to visualize.")
            return
        
        num_to_show = min(num_images, len(self.valid_images))
        step = max(1, len(self.valid_images) // num_to_show)
        
        for idx, image_path in enumerate(self.valid_images[::step][:num_to_show]):
            img = cv2.imread(image_path)
            if img is None:
                continue
            
            corners, gray = self.detect_chessboard(image_path)
            if corners is not None:
                # Draw corners
                cv2.drawChessboardCorners(img, self.chessboard_size, corners, True)
            
            cv2.imshow(f'Chessboard Detection ({idx+1}/{num_to_show})', img)
            cv2.waitKey(500)
        
        cv2.destroyAllWindows()

def main():
    # Calibration parameters
    IMAGE_DIR = 'bags/chess/'
    CHESSBOARD_WIDTH = 13
    CHESSBOARD_HEIGHT = 8
    CELL_SIZE_CM = 3.0
    OUTPUT_FILE = 'camera_calibration.pkl'
    
    # Create calibrator
    calibrator = ChessboardCalibrator(
        IMAGE_DIR,
        chessboard_size=(CHESSBOARD_WIDTH, CHESSBOARD_HEIGHT),
        cell_size_cm=CELL_SIZE_CM
    )
    
    # Perform calibration
    ret, mtx, dist, rvecs, tvecs = calibrator.calibrate()
    
    if ret:
        # Save calibration results
        calibrator.save_calibration(mtx, dist, OUTPUT_FILE)
        
        # Visualize results on sample images
        print("\nVisualizing chessboard detection on sample images...")
        calibrator.visualize_results(num_images=5)
    else:
        print("Calibration failed. Please check the images and try again.")

if __name__ == '__main__':
    main()
