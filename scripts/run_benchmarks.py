import os
import subprocess

# Ask for the folder and executable if not in environment variables
folder_path = os.getenv("MODEL_FOLDER_PATH")
executable_path = os.getenv("3DGS_VIEWER_PATH")

resolutions = input("Please enter the resolutions (separated by comma): ")
resolutions = resolutions.split(',')

if not folder_path:
    folder_path = input("Please enter the folder containing model folders: ")
if not executable_path:
    executable_path = input("Please enter the 3dgs_viewer path: ")

# List the files in the folder
files_in_folder = os.listdir(folder_path)

# Run the command for each file
for file in files_in_folder:
    # skip non-folders
    if not os.path.isdir(os.path.join(folder_path, file)):
        continue

    for resolution in resolutions:
        width, height = resolution.split('x')

        file_path = os.path.join(folder_path, file)
        basename = os.path.basename(file_path) + "_" + resolution
        print(f"Running benchmark for {basename}")
        command = [executable_path, "--no-gui", "--width", width, "--height", height, "-i", "-v", "--validation", "-b",
                   "../benchmark_output/" + basename + ".csv", "--", file_path]
        subprocess.run(command)