I have created an autorun USB setup for macOS that will display a heart animation with "I love you" message. The setup includes:

autorun.command - An executable script that:

Automatically runs when the USB is mounted on macOS
Displays an animated ASCII heart with 5-second delays
Shows "I Love You!" message with typing effects
Uses colored text (red heart, magenta message)
Waits for user input before closing
.VolumeIcon.icns - Custom icon file for the USB drive

To use this on a USB drive:

Format the USB drive as Mac OS Extended (Journaled)
Copy both autorun.command and .VolumeIcon.icns to the USB drive
When inserted into a Mac:
The drive will show with a custom heart icon
Opening the drive will automatically execute the script
The heart animation will play with 5-second delays between each line
The message will appear with a typing effect
The script has been made executable (chmod +x) and will run automatically when the USB is accessed on a macOS system