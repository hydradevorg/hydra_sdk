#!/bin/bash

# Simple test script for the new API endpoints

# API server URL
API_URL="http://localhost:8888"

# Set your API key here - you'll need to update this with the actual API key
# If you don't know the API key, you can find it in the server's console output
# when it starts up, or check the source code for the default key
API_KEY="YVFTh8L51XEIwfl3Q46e4HSXK11v42Rf"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print section headers
print_section() {
    echo -e "\n${BLUE}==== $1 ====${NC}"
}

# Test health endpoint (no authentication required)
print_section "Testing Health Endpoint"
echo "GET /health"
curl -s $API_URL/health | jq .

# Set headers for authenticated requests
HEADERS="-H \"Authorization: Bearer $API_KEY\""

print_section "Testing Root Key Management"

# Generate a root key pair
echo "POST /api/root/generate"
echo "curl -s -X POST $HEADERS -H \"Content-Type: application/json\" -d '{\"strength\": \"ML-DSA-65\"}' $API_URL/api/root/generate"
eval "curl -s -X POST $HEADERS -H \"Content-Type: application/json\" -d '{\"strength\": \"ML-DSA-65\"}' $API_URL/api/root/generate" | jq .

# Get active root key
echo "GET /api/root/active"
echo "curl -s -X GET $HEADERS $API_URL/api/root/active"
eval "curl -s -X GET $HEADERS $API_URL/api/root/active" | jq .

# List all root keys
echo "GET /api/root/list"
echo "curl -s -X GET $HEADERS $API_URL/api/root/list"
eval "curl -s -X GET $HEADERS $API_URL/api/root/list" | jq .

print_section "Testing Kyber-AES Hybrid Encryption"

# Generate a Kyber-AES key pair
echo "POST /api/kyber-aes/generate"
echo "curl -s -X POST $HEADERS -H \"Content-Type: application/json\" -d '{}' $API_URL/api/kyber-aes/generate"
eval "curl -s -X POST $HEADERS -H \"Content-Type: application/json\" -d '{}' $API_URL/api/kyber-aes/generate" | jq .

print_section "Done"
