#!/bin/bash

# API Client Example for Hydra Edge KMS
# This script demonstrates how to interact with the KMS API using curl

# Configuration
API_URL="http://localhost:8888"
API_KEY="YOUR_API_KEY_HERE"  # Replace with the actual API key

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper function for API calls
call_api() {
    local endpoint=$1
    local method=$2
    local data=$3
    
    echo -e "${BLUE}Making $method request to $endpoint${NC}"
    if [ -n "$data" ]; then
        echo -e "${BLUE}Request data:${NC} $data"
    fi
    
    response=$(curl -s -X "$method" \
        -H "Content-Type: application/json" \
        -H "Authorization: Bearer $API_KEY" \
        -d "$data" \
        "$API_URL$endpoint")
    
    echo -e "${BLUE}Response:${NC} $response"
    echo ""
    
    # Return the response for further processing
    echo "$response"
}

# Check if API server is running
check_health() {
    echo -e "${BLUE}Checking API server health...${NC}"
    response=$(call_api "/health" "GET")
    if [[ "$response" == *"\"status\":\"ok\""* ]]; then
        echo -e "${GREEN}API server is running!${NC}"
        return 0
    else
        echo -e "${RED}API server is not responding correctly.${NC}"
        return 1
    fi
}

# Generate Kyber KEM key pair
generate_kyber_keys() {
    echo -e "${BLUE}Generating Kyber KEM key pair...${NC}"
    response=$(call_api "/api/kyber/generate" "POST" "{}")
    
    # Extract and store keys
    public_key=$(echo "$response" | grep -o '"public_key":"[^"]*"' | cut -d'"' -f4)
    private_key=$(echo "$response" | grep -o '"private_key":"[^"]*"' | cut -d'"' -f4)
    
    echo -e "${GREEN}Kyber keys generated successfully!${NC}"
    echo "Public key: $public_key"
    echo "Private key: $private_key (keep this secret)"
    echo ""
    
    # Return keys for further use
    echo "$public_key:$private_key"
}

# Encapsulate a shared secret using Kyber
encapsulate_kyber() {
    local public_key=$1
    
    echo -e "${BLUE}Encapsulating shared secret with Kyber...${NC}"
    response=$(call_api "/api/kyber/encapsulate" "POST" "{\"public_key\":\"$public_key\"}")
    
    # Extract ciphertext and shared secret
    ciphertext=$(echo "$response" | grep -o '"ciphertext":"[^"]*"' | cut -d'"' -f4)
    shared_secret=$(echo "$response" | grep -o '"shared_secret":"[^"]*"' | cut -d'"' -f4)
    
    echo -e "${GREEN}Shared secret encapsulated successfully!${NC}"
    echo "Ciphertext: $ciphertext"
    echo "Shared secret: $shared_secret (keep this secret)"
    echo ""
    
    # Return ciphertext and shared secret
    echo "$ciphertext:$shared_secret"
}

# Decapsulate a shared secret using Kyber
decapsulate_kyber() {
    local private_key=$1
    local ciphertext=$2
    
    echo -e "${BLUE}Decapsulating shared secret with Kyber...${NC}"
    response=$(call_api "/api/kyber/decapsulate" "POST" "{\"private_key\":\"$private_key\",\"ciphertext\":\"$ciphertext\"}")
    
    # Extract shared secret
    shared_secret=$(echo "$response" | grep -o '"shared_secret":"[^"]*"' | cut -d'"' -f4)
    
    echo -e "${GREEN}Shared secret decapsulated successfully!${NC}"
    echo "Shared secret: $shared_secret (keep this secret)"
    echo ""
    
    # Return shared secret
    echo "$shared_secret"
}

# Generate Dilithium signature key pair
generate_dilithium_keys() {
    local strength=${1:-"ML-DSA-65"}  # Default to medium security
    
    echo -e "${BLUE}Generating Dilithium signature key pair (strength: $strength)...${NC}"
    response=$(call_api "/api/dilithium/generate" "POST" "{\"strength\":\"$strength\"}")
    
    # Extract and store keys
    public_key=$(echo "$response" | grep -o '"public_key":"[^"]*"' | cut -d'"' -f4)
    private_key=$(echo "$response" | grep -o '"private_key":"[^"]*"' | cut -d'"' -f4)
    
    echo -e "${GREEN}Dilithium keys generated successfully!${NC}"
    echo "Strength: $strength"
    echo "Public key: $public_key"
    echo "Private key: $private_key (keep this secret)"
    echo ""
    
    # Return keys for further use
    echo "$public_key:$private_key:$strength"
}

# Sign a message using Dilithium
sign_dilithium() {
    local private_key=$1
    local message=$2
    local strength=$3
    
    echo -e "${BLUE}Signing message with Dilithium...${NC}"
    echo "Message: $message"
    
    response=$(call_api "/api/dilithium/sign" "POST" \
        "{\"private_key\":\"$private_key\",\"message\":\"$message\",\"strength\":\"$strength\"}")
    
    # Extract signature
    signature=$(echo "$response" | grep -o '"signature":"[^"]*"' | cut -d'"' -f4)
    
    echo -e "${GREEN}Message signed successfully!${NC}"
    echo "Signature: $signature"
    echo ""
    
    # Return signature
    echo "$signature"
}

# Verify a signature using Dilithium
verify_dilithium() {
    local public_key=$1
    local message=$2
    local signature=$3
    local strength=$4
    
    echo -e "${BLUE}Verifying Dilithium signature...${NC}"
    echo "Message: $message"
    
    response=$(call_api "/api/dilithium/verify" "POST" \
        "{\"public_key\":\"$public_key\",\"message\":\"$message\",\"signature\":\"$signature\",\"strength\":\"$strength\"}")
    
    # Check verification result
    valid=$(echo "$response" | grep -o '"valid":\(true\|false\)' | cut -d':' -f2)
    
    if [ "$valid" = "true" ]; then
        echo -e "${GREEN}Signature is valid!${NC}"
    else
        echo -e "${RED}Signature is invalid!${NC}"
    fi
    echo ""
    
    # Return verification result
    echo "$valid"
}

# Main demo function
run_demo() {
    # Check if API server is running
    check_health
    if [ $? -ne 0 ]; then
        echo -e "${RED}Cannot connect to API server. Make sure it's running and try again.${NC}"
        exit 1
    fi
    
    echo -e "${BLUE}=== Kyber KEM Demo ===${NC}"
    # Generate Kyber keys
    kyber_keys=$(generate_kyber_keys)
    kyber_public_key=$(echo "$kyber_keys" | cut -d':' -f1)
    kyber_private_key=$(echo "$kyber_keys" | cut -d':' -f2)
    
    # Encapsulate a shared secret
    encap_result=$(encapsulate_kyber "$kyber_public_key")
    ciphertext=$(echo "$encap_result" | cut -d':' -f1)
    encapsulated_secret=$(echo "$encap_result" | cut -d':' -f2)
    
    # Decapsulate the shared secret
    decapsulated_secret=$(decapsulate_kyber "$kyber_private_key" "$ciphertext")
    
    # Verify that the shared secrets match
    if [ "$encapsulated_secret" = "$decapsulated_secret" ]; then
        echo -e "${GREEN}Success! Encapsulated and decapsulated shared secrets match.${NC}"
    else
        echo -e "${RED}Error! Shared secrets do not match.${NC}"
    fi
    
    echo -e "${BLUE}=== Dilithium Signature Demo ===${NC}"
    # Generate Dilithium keys
    dilithium_keys=$(generate_dilithium_keys "ML-DSA-65")
    dilithium_public_key=$(echo "$dilithium_keys" | cut -d':' -f1)
    dilithium_private_key=$(echo "$dilithium_keys" | cut -d':' -f2)
    dilithium_strength=$(echo "$dilithium_keys" | cut -d':' -f3)
    
    # Sign a message
    message="This is a test message for Dilithium signature verification."
    signature=$(sign_dilithium "$dilithium_private_key" "$message" "$dilithium_strength")
    
    # Verify the signature
    verify_dilithium "$dilithium_public_key" "$message" "$signature" "$dilithium_strength"
    
    # Try to verify with a modified message
    modified_message="This is a MODIFIED message for Dilithium signature verification."
    echo -e "${BLUE}Testing with modified message...${NC}"
    verify_dilithium "$dilithium_public_key" "$modified_message" "$signature" "$dilithium_strength"
}

# Run the demo
run_demo
