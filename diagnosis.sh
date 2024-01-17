#!/bin/bash
killall opencv_example_frontend-x86_64.AppImage
killall opencv_example_sweeper-x86_64.AppImage
gnome-terminal -x ./opencv_example_frontend-x86_64.AppImage
gnome-terminal -x ./opencv_example_sweeper-x86_64.AppImage