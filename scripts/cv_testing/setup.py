from setuptools import find_packages, setup

package_name = "cv_testing"

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
    description="Image replay publisher for /camera/img",
    license="Apache-2.0",
    entry_points={
        "console_scripts": [
            "cv_testing_node = cv_testing.cv_testing_node:main",
        ],
    },
)
