#!/bin/bash

# Clear the screen
clear

# Function to print with delay
print_with_delay() {
    text=$1
    for ((i=0; i<${#text}; i++)); do
        echo -n "${text:$i:1}"
        sleep 0.1
    done
    echo
    sleep 5
}

# Set text color to red
echo -e "\033[31m"

# Draw heart with animation
echo "    Creating heart for you..."
sleep 5
clear

# Heart ASCII art with animation
print_with_delay "    ♥♥   ♥♥"
print_with_delay "  ♥♥♥♥♥♥♥♥♥♥"
print_with_delay " ♥♥♥♥♥♥♥♥♥♥♥♥"
print_with_delay " ♥♥♥♥♥♥♥♥♥♥♥♥"
print_with_delay "  ♥♥♥♥♥♥♥♥♥♥"
print_with_delay "    ♥♥♥♥♥♥"
print_with_delay "      ♥♥"

# Set text color to magenta
echo -e "\033[35m"
sleep 5

# Print message with typing effect
print_with_delay "       I Love You! ❤️"

# Reset text color
echo -e "\033[0m"

# Keep window open
read -p "Press [Enter] to close..."
