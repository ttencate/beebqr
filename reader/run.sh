#!/bin/bash
javac Reader.java && java -XX:-OmitStackTraceInFastThrow Reader $*
