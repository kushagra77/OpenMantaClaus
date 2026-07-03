#!/usr/bin/env python3
"""
Convert camera calibration pickle file to CSV format for use in OpenCV C++.
Reads camera_calibration.pkl and exports camera matrix and distortion coefficients to CSV.
"""

import pickle
import csv
import numpy as np
import os

def pkl_to_csv(pkl_file='camera_calibration.pkl', output_dir='calibration_data'):
    """
    Convert calibration pickle file to CSV files.
    
    Args:
        pkl_file: Path to the pickle file
        output_dir: Directory to save CSV files
    """
    
    # Check if pickle file exists
    if not os.path.exists(pkl_file):
        print(f"Error: {pkl_file} not found.")
        return False
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Load pickle file
    try:
        with open(pkl_file, 'rb') as f:
            calibration_data = pickle.load(f)
        print(f"✓ Loaded calibration data from {pkl_file}")
    except Exception as e:
        print(f"Error loading pickle file: {e}")
        return False
    
    # Extract data
    camera_matrix = calibration_data.get('camera_matrix')
    distortion_coeffs = calibration_data.get('distortion_coefficients')
    image_size = calibration_data.get('image_size')
    chessboard_size = calibration_data.get('chessboard_size')
    cell_size_m = calibration_data.get('cell_size_m')
    valid_images = calibration_data.get('valid_images')
    
    if camera_matrix is None or distortion_coeffs is None:
        print("Error: Camera matrix or distortion coefficients not found in pickle file.")
        return False
    
    # Save camera matrix
    camera_matrix_file = os.path.join(output_dir, 'camera_matrix.csv')
    try:
        np.savetxt(camera_matrix_file, camera_matrix, delimiter=',', fmt='%.6f')
        print(f"✓ Saved camera matrix to {camera_matrix_file}")
    except Exception as e:
        print(f"Error saving camera matrix: {e}")
        return False
    
    # Save distortion coefficients (flatten if needed)
    dist_coeffs_file = os.path.join(output_dir, 'distortion_coefficients.csv')
    try:
        dist_flat = distortion_coeffs.flatten()
        # Save as a single row
        np.savetxt(dist_coeffs_file, [dist_flat], delimiter=',', fmt='%.6f')
        print(f"✓ Saved distortion coefficients to {dist_coeffs_file}")
    except Exception as e:
        print(f"Error saving distortion coefficients: {e}")
        return False
    
    # Save metadata
    metadata_file = os.path.join(output_dir, 'calibration_metadata.csv')
    try:
        with open(metadata_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['Parameter', 'Value'])
            writer.writerow(['Image Width', image_size[0] if image_size else 'N/A'])
            writer.writerow(['Image Height', image_size[1] if image_size else 'N/A'])
            writer.writerow(['Chessboard Width (cells)', chessboard_size[0] if chessboard_size else 'N/A'])
            writer.writerow(['Chessboard Height (cells)', chessboard_size[1] if chessboard_size else 'N/A'])
            writer.writerow(['Cell Size (m)', cell_size_m if cell_size_m else 'N/A'])
            writer.writerow(['Number of Valid Images', len(valid_images) if valid_images else 'N/A'])
        print(f"✓ Saved metadata to {metadata_file}")
    except Exception as e:
        print(f"Error saving metadata: {e}")
        return False
    
    # Save combined calibration file (camera matrix + distortion coefficients)
    combined_file = os.path.join(output_dir, 'calibration.csv')
    try:
        with open(combined_file, 'w', newline='') as f:
            writer = csv.writer(f)
            # Write camera matrix
            writer.writerow(['CAMERA_MATRIX'])
            for row in camera_matrix:
                writer.writerow(row)
            writer.writerow([])  # Empty row as separator
            # Write distortion coefficients
            writer.writerow(['DISTORTION_COEFFICIENTS'])
            writer.writerow(distortion_coeffs.flatten())
        print(f"✓ Saved combined calibration to {combined_file}")
    except Exception as e:
        print(f"Error saving combined calibration: {e}")
        return False
    
    # Print summary
    print("\n" + "="*60)
    print("CALIBRATION SUMMARY")
    print("="*60)
    print(f"Camera Matrix:")
    print(camera_matrix)
    print(f"\nDistortion Coefficients:")
    print(distortion_coeffs.flatten())
    if image_size:
        print(f"\nImage Size: {image_size[0]}x{image_size[1]}")
    if chessboard_size:
        print(f"Chessboard: {chessboard_size[0]}x{chessboard_size[1]} cells")
    if cell_size_m:
        print(f"Cell Size: {cell_size_m*100:.1f} cm")
    if valid_images:
        print(f"Number of images used: {len(valid_images)}")
    print("="*60)
    
    print(f"\n✓ All files saved to '{output_dir}/' directory")
    return True

def main():
    pkl_to_csv('calibration_data/camera_calibration.pkl', 'calibration_data')

if __name__ == '__main__':
    main()
