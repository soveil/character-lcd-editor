arduino-cli compile --fqbn arduino:avr:uno $1
arduino-cli upload -p $2 --fqbn arduino:avr:uno $1
