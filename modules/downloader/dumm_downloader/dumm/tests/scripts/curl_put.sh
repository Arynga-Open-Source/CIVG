#!/bin/bash

curl -X PUT  --data-binary   @${1} http://127.0.0.1:8080/files/`basename ${1}`