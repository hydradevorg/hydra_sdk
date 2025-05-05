#!/bin/bash

# API URL and key
API_URL="http://localhost:8888"
API_KEY="sABUcL2SsG9EcdvkLTkiMLxpooCzCcGD"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Debugging Hydra Edge KMS API Server${NC}"
echo "API URL: $API_URL"
echo "API Key: $API_KEY"
echo ""

# Test health endpoint
echo -e "${BLUE}Testing Health Endpoint${NC}"
curl -s "$API_URL/health" | jq
echo ""

# Test Kyber KEM key generation
echo -e "${BLUE}Testing Kyber KEM Key Generation${NC}"
kyber_response=$(curl -s -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $API_KEY" \
  -d "{}" \
  "$API_URL/api/kyber/generate")

# Check if the response is valid JSON
if echo "$kyber_response" | jq . > /dev/null 2>&1; then
    echo -e "${GREEN}Response is valid JSON${NC}"
    # Extract and display only the first 50 characters of each key
    public_key=$(echo "$kyber_response" | jq -r '.public_key' | cut -c 1-50)
    private_key=$(echo "$kyber_response" | jq -r '.private_key' | cut -c 1-50)
    echo "Public Key (truncated): $public_key..."
    echo "Private Key (truncated): $private_key..."
    
    # Store the full keys for later use
    full_public_key=$(echo "$kyber_response" | jq -r '.public_key')
    full_private_key=$(echo "$kyber_response" | jq -r '.private_key')
else
    echo -e "${RED}Invalid JSON response${NC}"
    echo "$kyber_response"
fi
echo ""

# Test Kyber KEM encapsulation
echo -e "${BLUE}Testing Kyber KEM Encapsulation${NC}"
if [ -n "$full_public_key" ]; then
    encap_response=$(curl -s -X POST \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $API_KEY" \
      -d "{\"public_key\": \"$full_public_key\"}" \
      "$API_URL/api/kyber/encapsulate")
    
    if echo "$encap_response" | jq . > /dev/null 2>&1; then
        echo -e "${GREEN}Response is valid JSON${NC}"
        ciphertext=$(echo "$encap_response" | jq -r '.ciphertext' | cut -c 1-50)
        shared_secret=$(echo "$encap_response" | jq -r '.shared_secret' | cut -c 1-50)
        echo "Ciphertext (truncated): $ciphertext..."
        echo "Shared Secret (truncated): $shared_secret..."
        
        # Store the full values for later use
        full_ciphertext=$(echo "$encap_response" | jq -r '.ciphertext')
        full_shared_secret_enc=$(echo "$encap_response" | jq -r '.shared_secret')
    else
        echo -e "${RED}Invalid JSON response${NC}"
        echo "$encap_response"
    fi
else
    echo -e "${RED}Skipping encapsulation test - no public key available${NC}"
fi
echo ""

# Test Kyber KEM decapsulation
echo -e "${BLUE}Testing Kyber KEM Decapsulation${NC}"
if [ -n "$full_private_key" ] && [ -n "$full_ciphertext" ]; then
    decap_response=$(curl -s -X POST \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $API_KEY" \
      -d "{\"private_key\": \"$full_private_key\", \"ciphertext\": \"$full_ciphertext\"}" \
      "$API_URL/api/kyber/decapsulate")
    
    if echo "$decap_response" | jq . > /dev/null 2>&1; then
        echo -e "${GREEN}Response is valid JSON${NC}"
        shared_secret_dec=$(echo "$decap_response" | jq -r '.shared_secret' | cut -c 1-50)
        echo "Shared Secret (truncated): $shared_secret_dec..."
        
        # Compare shared secrets
        if [ "$full_shared_secret_enc" = "$(echo "$decap_response" | jq -r '.shared_secret')" ]; then
            echo -e "${GREEN} Kyber KEM test passed: Shared secrets match${NC}"
        else
            echo -e "${RED} Kyber KEM test failed: Shared secrets do not match${NC}"
        fi
    else
        echo -e "${RED}Invalid JSON response${NC}"
        echo "$decap_response"
    fi
else
    echo -e "${RED}Skipping decapsulation test - no private key or ciphertext available${NC}"
fi
echo ""

# Test Dilithium signature generation
echo -e "${BLUE}Testing Dilithium Signature Generation${NC}"
dilithium_response=$(curl -s -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $API_KEY" \
  -d "{\"strength\": \"ML-DSA-65\"}" \
  "$API_URL/api/dilithium/generate")

if echo "$dilithium_response" | jq . > /dev/null 2>&1; then
    echo -e "${GREEN}Response is valid JSON${NC}"
    public_key=$(echo "$dilithium_response" | jq -r '.public_key' | cut -c 1-50)
    private_key=$(echo "$dilithium_response" | jq -r '.private_key' | cut -c 1-50)
    strength=$(echo "$dilithium_response" | jq -r '.strength')
    echo "Public Key (truncated): $public_key..."
    echo "Private Key (truncated): $private_key..."
    echo "Strength: $strength"
    
    # Store the full keys for later use
    full_dil_public_key=$(echo "$dilithium_response" | jq -r '.public_key')
    full_dil_private_key=$(echo "$dilithium_response" | jq -r '.private_key')
else
    echo -e "${RED}Invalid JSON response${NC}"
    echo "$dilithium_response"
fi
echo ""

# Test Dilithium message signing
echo -e "${BLUE}Testing Dilithium Message Signing${NC}"
if [ -n "$full_dil_private_key" ]; then
    message="This is a test message to sign"
    sign_response=$(curl -s -X POST \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $API_KEY" \
      -d "{\"private_key\": \"$full_dil_private_key\", \"message\": \"$message\", \"strength\": \"$strength\"}" \
      "$API_URL/api/dilithium/sign")
    
    if echo "$sign_response" | jq . > /dev/null 2>&1; then
        echo -e "${GREEN}Response is valid JSON${NC}"
        signature=$(echo "$sign_response" | jq -r '.signature' | cut -c 1-50)
        echo "Signature (truncated): $signature..."
        
        # Store the full signature for later use
        full_signature=$(echo "$sign_response" | jq -r '.signature')
    else
        echo -e "${RED}Invalid JSON response${NC}"
        echo "$sign_response"
    fi
else
    echo -e "${RED}Skipping signing test - no private key available${NC}"
fi
echo ""

# Test Dilithium signature verification
echo -e "${BLUE}Testing Dilithium Signature Verification${NC}"
if [ -n "$full_dil_public_key" ] && [ -n "$full_signature" ]; then
    verify_response=$(curl -s -X POST \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $API_KEY" \
      -d "{\"public_key\": \"$full_dil_public_key\", \"message\": \"$message\", \"signature\": \"$full_signature\", \"strength\": \"$strength\"}" \
      "$API_URL/api/dilithium/verify")
    
    if echo "$verify_response" | jq . > /dev/null 2>&1; then
        echo -e "${GREEN}Response is valid JSON${NC}"
        is_valid=$(echo "$verify_response" | jq -r '.valid')
        echo "Signature Valid: $is_valid"
        
        # Check if signature is valid
        if [ "$is_valid" = "true" ]; then
            echo -e "${GREEN} Dilithium signature test passed: Signature is valid${NC}"
        else
            echo -e "${RED} Dilithium signature test failed: Signature is invalid${NC}"
        fi
    else
        echo -e "${RED}Invalid JSON response${NC}"
        echo "$verify_response"
    fi
else
    echo -e "${RED}Skipping verification test - no public key or signature available${NC}"
fi
echo ""

echo -e "${BLUE}API debugging completed${NC}"
