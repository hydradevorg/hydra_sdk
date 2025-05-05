#!/bin/bash

# Get the API key from the command line arguments
API_KEY=$1
if [ -z "$API_KEY" ]; then
    echo "Please provide the API key as a command line argument"
    echo "Usage: $0 <api_key>"
    exit 1
fi

# API URL
API_URL="http://localhost:8888"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Testing Hydra Edge KMS API Server${NC}"
echo "API URL: $API_URL"
echo "API Key: $API_KEY"
echo ""

# Function to make API calls
call_api() {
    local endpoint=$1
    local method=$2
    local data=$3
    
    echo -e "${BLUE}Testing $method $endpoint${NC}"
    
    if [ "$method" == "GET" ]; then
        response=$(curl -s -X $method \
            -H "Authorization: Bearer $API_KEY" \
            "$API_URL$endpoint")
    else
        response=$(curl -s -X $method \
            -H "Content-Type: application/json" \
            -H "Authorization: Bearer $API_KEY" \
            -d "$data" \
            "$API_URL$endpoint")
    fi
    
    # Check if the response is valid JSON
    if echo "$response" | jq . > /dev/null 2>&1; then
        echo -e "${GREEN}Response:${NC}"
        echo "$response" | jq .
        echo ""
        return 0
    else
        echo -e "${RED}Error:${NC}"
        echo "$response"
        echo ""
        return 1
    fi
}

# Test health endpoint
call_api "/health" "GET"

# Test Kyber KEM key generation
echo -e "${BLUE}Testing Kyber KEM key generation and encapsulation${NC}"
kyber_response=$(call_api "/api/kyber/generate" "POST" "{}")
if [ $? -eq 0 ]; then
    # Extract public and private keys
    public_key=$(echo "$kyber_response" | jq -r '.public_key')
    private_key=$(echo "$kyber_response" | jq -r '.private_key')
    
    # Test encapsulation
    encap_response=$(call_api "/api/kyber/encapsulate" "POST" "{\"public_key\": \"$public_key\"}")
    if [ $? -eq 0 ]; then
        # Extract ciphertext and shared secret
        ciphertext=$(echo "$encap_response" | jq -r '.ciphertext')
        shared_secret_enc=$(echo "$encap_response" | jq -r '.shared_secret')
        
        # Test decapsulation
        decap_response=$(call_api "/api/kyber/decapsulate" "POST" "{\"private_key\": \"$private_key\", \"ciphertext\": \"$ciphertext\"}")
        if [ $? -eq 0 ]; then
            shared_secret_dec=$(echo "$decap_response" | jq -r '.shared_secret')
            
            # Compare shared secrets
            if [ "$shared_secret_enc" == "$shared_secret_dec" ]; then
                echo -e "${GREEN}✓ Kyber KEM test passed: Shared secrets match${NC}"
            else
                echo -e "${RED}✗ Kyber KEM test failed: Shared secrets do not match${NC}"
            fi
        fi
    fi
fi

# Test Dilithium signature generation
echo -e "${BLUE}Testing Dilithium signature generation and verification${NC}"
dilithium_response=$(call_api "/api/dilithium/generate" "POST" "{\"strength\": \"ML-DSA-65\"}")
if [ $? -eq 0 ]; then
    # Extract public and private keys
    public_key=$(echo "$dilithium_response" | jq -r '.public_key')
    private_key=$(echo "$dilithium_response" | jq -r '.private_key')
    strength=$(echo "$dilithium_response" | jq -r '.strength')
    
    # Test message signing
    message="This is a test message to sign"
    sign_response=$(call_api "/api/dilithium/sign" "POST" "{\"private_key\": \"$private_key\", \"message\": \"$message\", \"strength\": \"$strength\"}")
    if [ $? -eq 0 ]; then
        # Extract signature
        signature=$(echo "$sign_response" | jq -r '.signature')
        
        # Test signature verification
        verify_response=$(call_api "/api/dilithium/verify" "POST" "{\"public_key\": \"$public_key\", \"message\": \"$message\", \"signature\": \"$signature\", \"strength\": \"$strength\"}")
        if [ $? -eq 0 ]; then
            is_valid=$(echo "$verify_response" | jq -r '.valid')
            
            # Check if signature is valid
            if [ "$is_valid" == "true" ]; then
                echo -e "${GREEN}✓ Dilithium signature test passed: Signature is valid${NC}"
            else
                echo -e "${RED}✗ Dilithium signature test failed: Signature is invalid${NC}"
            fi
        fi
    fi
fi

echo -e "${BLUE}API testing completed${NC}"
