from setuptools import find_packages, setup

package_name = "cv"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="kushion",
    maintainer_email="you@example.com",
    description="Computer Vision package (ament_python port)",
    license="Apache-2.0",
    entry_points={
        "console_scripts": [
            "cv_node = cv.cv_node:main",
            "testing_node = cv.testing_node:main",
            "bottom_cv_node = cv.bottom_cv_node:main"
        ],
    },
)