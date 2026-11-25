#!/bin/bash


rm -f *.zip


zip -r code.zip examples/ src/ -x "*credentials.h"
