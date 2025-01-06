#!/bin/bash

# Script to test a simple REST API with curl
# Steps:
# 1. Create a JSON resource using POST
# 2. Retrieve the same JSON resource using GET

set -xe

# Base URL of the API
BASE_URL="http://localhost:8080"

# Resource endpoint
RESOURCE_ENDPOINT="/resource"

# JSON data to be sent in the POST request
JSON_DATA='{"name": "Test Resource", "value": 42}'

# Step 1: Create a resource with a POST request
echo "Creating a resource with POST request..."
CREATE_RESPONSE=$(curl -s -X POST "$BASE_URL$RESOURCE_ENDPOINT" \
  -H "Content-Type: application/json" \
  -d "$JSON_DATA")

echo "POST Response: $CREATE_RESPONSE"

# Step 2: Retrieve the created resource with a GET request
echo "Retrieving the created resource with GET request..."
GET_RESPONSE=$(curl -s -X GET "$BASE_URL$RESOURCE_ENDPOINT" \
  -H "Accept: application/json")

echo "GET Response: $GET_RESPONSE"

# Validate the retrieved resource (optional)
if [ "$CREATE_RESPONSE" == "$GET_RESPONSE" ]; then
  echo "Test passed: Retrieved resource matches the created one."
else
  echo "Test failed: Retrieved resource does not match the created one."
fi
