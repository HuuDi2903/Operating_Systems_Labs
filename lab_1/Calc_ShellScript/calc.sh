#!/usr/bin/bash

# File to store ANS value
ANS_FILE="ans.txt"
HIST_FILE="history.txt"

# Initialize ANS if file doesn't exist
if [ ! -f "$ANS_FILE" ]; then
    echo "0" > "$ANS_FILE"
fi

# Initialize history file if it doesn't exist
if [ ! -f "$HIST_FILE" ]; then
    touch "$HIST_FILE"
fi

# Function to add calculation to history
add_to_history() {
    local expression="$1"
    local result="$2"
    echo "$expression = $result" >> "$HIST_FILE"
    # Keep only last 5 entries
    tail -n 5 "$HIST_FILE" > "$HIST_FILE.tmp"
    mv "$HIST_FILE.tmp" "$HIST_FILE"
}

# Function to perform calculation
calculate() {
    local num1="$1"
    local operator="$2"
    local num2="$3"
    local result

    # Check if any parameter is empty
    if [ -z "$num1" ] || [ -z "$operator" ] || [ -z "$num2" ]; then
        echo "SYNTAX ERROR"
        return 1
    fi

    # Check if num1 and num2 are valid numbers (allowing for decimals and negative numbers)
    # Skip this check if the input is "ANS"
    if [ "$num1" != "ANS" ] && ! [[ "$num1" =~ ^-?[0-9]+(\.?[0-9]*)?$ ]]; then
        echo "SYNTAX ERROR"
        return 1
    fi
    if [ "$num2" != "ANS" ] && ! [[ "$num2" =~ ^-?[0-9]+(\.?[0-9]*)?$ ]]; then
        echo "SYNTAX ERROR"
        return 1
    fi

    # Check if operator is valid
    if ! [[ "$operator" =~ ^[+\-x/%]$ ]]; then
        echo "SYNTAX ERROR"
        return 1
    fi

    case $operator in
        "+")
            result=$(echo "$num1 + $num2" | bc -l)
            ;;
        "-")
            result=$(echo "$num1 - $num2" | bc -l)
            ;;
        "x")
            result=$(echo "$num1 * $num2" | bc -l)
            ;;
        "/")
            if [ "$num2" = "0" ]; then
                echo "MATH ERROR"
                return 1
            fi
            result=$(echo "scale=2; $num1 / $num2" | bc -l)
            ;;
        "%")
            if [ "$num2" = "0" ]; then
                echo "MATH ERROR"
                return 1
            fi
            result=$(echo "$num1 % $num2" | bc)
            ;;
        *)
            echo "SYNTAX ERROR"
            return 1
            ;;
    esac

    # Round to 2 decimal places if necessary
    printf "%.2f\n" "$result"
    echo "$result" > "$ANS_FILE"
    add_to_history "$num1 $operator $num2" "$result"
}

# Main loop
while true; do
    clear
    echo -n ">> "
    read -r input

    # Exit condition
    if [ "$input" = "EXIT" ]; then
        break
    fi

    # Show history
    if [ "$input" = "HIST" ]; then
        cat "$HIST_FILE"
        read -n 1
        continue
    fi

    # Parse input
    read -r num1 operator num2 <<< "$input"

    # Replace ANS with its value
    if [ "$num1" = "ANS" ]; then
        num1=$(cat "$ANS_FILE")
    fi
    if [ "$num2" = "ANS" ]; then
        num2=$(cat "$ANS_FILE")
    fi

    # Validate input
    if ! [[ "$num1" =~ ^-?[0-9]+\.?[0-9]*$ ]] || ! [[ "$num2" =~ ^-?[0-9]+\.?[0-9]*$ ]]; then
        echo "SYNTAX ERROR"
        read -n 1
        continue
    fi

    # Perform calculation
    result=$(calculate "$num1" "$operator" "$num2")
    echo "$result"
    
    # Wait for key press before next calculation
    read -n 1
done


