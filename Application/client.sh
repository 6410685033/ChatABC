#!/bin/bash

# Compile the Java files
javac -d out src/*.java

# Run the ChatClientGUI class
java -cp out ChatClientGUI
