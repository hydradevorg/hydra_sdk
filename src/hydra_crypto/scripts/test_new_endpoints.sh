#!/bin/bash

# Test script for the new API endpoints
# This script tests the root key management and Kyber-AES hybrid encryption functionality

# API server URL
API_URL="http://localhost:8888"

# First, let's try to make a request without an API key to see if we get the API key in the error message
echo "Attempting to discover API key..."
API_KEY_RESPONSE=$(curl -s -X GET $API_URL/api/kyber/generate)
echo "Response: $API_KEY_RESPONSE"

# Extract API key from the error message if possible
if [[ "$API_KEY_RESPONSE" == *"API key"* ]]; then
    # Try to extract the API key from the error message
    # This is just a placeholder - the actual extraction depends on the error format
    echo "API key information found in response. Please check the response and update the API_KEY variable below."
fi

# Set your API key here - you'll need to update this with the actual API key
API_KEY="your_api_key_here"

# Set headers for API requests
HEADERS="Authorization: Bearer $API_KEY"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print section headers
print_section() {
    echo -e "\n${BLUE}==== $1 ====${NC}"
}

# Function to check if a command was successful
check_success() {
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}SUCCESS${NC}: $1"
    else
        echo -e "${RED}FAILED${NC}: $1"
        exit 1
    fi
}

# Function to make API calls and store the response
call_api() {
    local method=$1
    local endpoint=$2
    local data=$3
    local output_file=$4

    if [ -z "$data" ]; then
        curl -s -X $method -H "$HEADERS" -H "Content-Type: application/json" $API_URL$endpoint > $output_file
    else
        curl -s -X $method -H "$HEADERS" -H "Content-Type: application/json" -d "$data" $API_URL$endpoint > $output_file
    fi

    # Check if the response contains an error
    if grep -q "error" $output_file; then
        echo -e "${RED}API ERROR${NC}: $(cat $output_file)"
        return 1
    fi
    return 0
}

# Create a temporary directory for test files
TMP_DIR=$(mktemp -d)
echo "Using temporary directory: $TMP_DIR"

# Test health endpoint
print_section "Testing Health Endpoint"
call_api "GET" "/health" "" "$TMP_DIR/health.json"
check_success "Health check"
cat "$TMP_DIR/health.json" | jq .

# Test Root Key Management
print_section "Testing Root Key Management"

# Generate a root key pair
echo "Generating root key pair..."
call_api "POST" "/api/root/generate" '{"strength": "ML-DSA-65"}' "$TMP_DIR/root_key.json"
check_success "Generate root key pair"
cat "$TMP_DIR/root_key.json" | jq .

# Get the root key ID
ROOT_KEY_ID=$(cat "$TMP_DIR/root_key.json" | jq -r '.id')
echo "Root key ID: $ROOT_KEY_ID"

# Get active root key
echo "Getting active root key..."
call_api "GET" "/api/root/active" "" "$TMP_DIR/active_root_key.json"
check_success "Get active root key"
cat "$TMP_DIR/active_root_key.json" | jq .

# List all root keys
echo "Listing all root keys..."
call_api "GET" "/api/root/list" "" "$TMP_DIR/root_keys_list.json"
check_success "List root keys"
cat "$TMP_DIR/root_keys_list.json" | jq .

# Test Kyber-AES Hybrid Encryption
print_section "Testing Kyber-AES Hybrid Encryption"

# Generate a Kyber-AES key pair
echo "Generating Kyber-AES key pair..."
call_api "POST" "/api/kyber-aes/generate" "{}" "$TMP_DIR/kyber_aes_keys.json"
check_success "Generate Kyber-AES key pair"
cat "$TMP_DIR/kyber_aes_keys.json" | jq .

# Extract the public and private keys
PUBLIC_KEY=$(cat "$TMP_DIR/kyber_aes_keys.json" | jq -r '.public_key')
PRIVATE_KEY=$(cat "$TMP_DIR/kyber_aes_keys.json" | jq -r '.private_key')
echo "Public key length: ${#PUBLIC_KEY}"
echo "Private key length: ${#PRIVATE_KEY}"

# Sign the public key with the root key
echo "Signing public key..."
call_api "POST" "/api/root/sign" "{\"public_key\": \"$PUBLIC_KEY\"}" "$TMP_DIR/signed_key.json"
check_success "Sign public key"
cat "$TMP_DIR/signed_key.json" | jq .

# Extract the signature
SIGNATURE=$(cat "$TMP_DIR/signed_key.json" | jq -r '.signature')
echo "Signature length: ${#SIGNATURE}"

# Verify the signature
echo "Verifying signature..."
call_api "POST" "/api/root/verify" "{\"public_key\": \"$PUBLIC_KEY\", \"signature\": \"$SIGNATURE\"}" "$TMP_DIR/verify_result.json"
check_success "Verify signature"
cat "$TMP_DIR/verify_result.json" | jq .

# Encrypt the public key
echo "Encrypting public key..."
call_api "POST" "/api/root/encrypt" "{\"public_key\": \"$PUBLIC_KEY\"}" "$TMP_DIR/encrypted_key.json"
check_success "Encrypt public key"
cat "$TMP_DIR/encrypted_key.json" | jq .

# Extract the encrypted key
ENCRYPTED_KEY=$(cat "$TMP_DIR/encrypted_key.json" | jq -r '.encrypted_public_key')
echo "Encrypted key length: ${#ENCRYPTED_KEY}"

# Decrypt the encrypted key
echo "Decrypting encrypted key..."
call_api "POST" "/api/root/decrypt" "{\"encrypted_public_key\": \"$ENCRYPTED_KEY\"}" "$TMP_DIR/decrypted_key.json"
check_success "Decrypt public key"
cat "$TMP_DIR/decrypted_key.json" | jq .

# Verify the decrypted key matches the original
DECRYPTED_KEY=$(cat "$TMP_DIR/decrypted_key.json" | jq -r '.public_key')
if [ "$DECRYPTED_KEY" == "$PUBLIC_KEY" ]; then
    echo -e "${GREEN}SUCCESS${NC}: Decrypted key matches original"
else
    echo -e "${RED}FAILED${NC}: Decrypted key does not match original"
    exit 1
fi

# Test Kyber-AES encryption and decryption
print_section "Testing Kyber-AES Encryption/Decryption"

# Sample plaintext
PLAINTEXT="This is a secret message that needs to be encrypted securely using post-quantum cryptography."
echo "Original plaintext: $PLAINTEXT"

# Encrypt the plaintext
echo "Encrypting plaintext..."
call_api "POST" "/api/kyber-aes/encrypt" "{\"public_key\": \"$PUBLIC_KEY\", \"plaintext\": \"$PLAINTEXT\"}" "$TMP_DIR/encrypted_data.json"
check_success "Encrypt data"
cat "$TMP_DIR/encrypted_data.json" | jq .

# Extract the ciphertext
CIPHERTEXT=$(cat "$TMP_DIR/encrypted_data.json" | jq -r '.ciphertext')
echo "Ciphertext length: ${#CIPHERTEXT}"

# Decrypt the ciphertext
echo "Decrypting ciphertext..."
call_api "POST" "/api/kyber-aes/decrypt" "{\"private_key\": \"$PRIVATE_KEY\", \"ciphertext\": \"$CIPHERTEXT\"}" "$TMP_DIR/decrypted_data.json"
check_success "Decrypt data"
cat "$TMP_DIR/decrypted_data.json" | jq .

# Verify the decrypted plaintext matches the original
DECRYPTED_PLAINTEXT=$(cat "$TMP_DIR/decrypted_data.json" | jq -r '.plaintext')
if [ "$DECRYPTED_PLAINTEXT" == "$PLAINTEXT" ]; then
    echo -e "${GREEN}SUCCESS${NC}: Decrypted plaintext matches original"
else
    echo -e "${RED}FAILED${NC}: Decrypted plaintext does not match original"
    exit 1
fi

# Test key rotation
print_section "Testing Key Rotation"

# Rotate the root key
echo "Rotating root key..."
call_api "POST" "/api/root/rotate" '{"force": true}' "$TMP_DIR/rotated_key.json"
check_success "Rotate root key"
cat "$TMP_DIR/rotated_key.json" | jq .

# Get the new root key ID
NEW_ROOT_KEY_ID=$(cat "$TMP_DIR/rotated_key.json" | jq -r '.id')
echo "New root key ID: $NEW_ROOT_KEY_ID"

# Verify the new key is different from the old one
if [ "$NEW_ROOT_KEY_ID" != "$ROOT_KEY_ID" ]; then
    echo -e "${GREEN}SUCCESS${NC}: Root key was rotated successfully"
else
    echo -e "${RED}FAILED${NC}: Root key rotation did not generate a new key"
    exit 1
fi

# List all root keys again to verify both keys are present
echo "Listing all root keys after rotation..."
call_api "GET" "/api/root/list" "" "$TMP_DIR/root_keys_list_after.json"
check_success "List root keys after rotation"
cat "$TMP_DIR/root_keys_list_after.json" | jq .

# Check if the active key is the new one
ACTIVE_KEY_ID=$(cat "$TMP_DIR/active_root_key.json" | jq -r '.id')
if [ "$ACTIVE_KEY_ID" == "$NEW_ROOT_KEY_ID" ]; then
    echo -e "${GREEN}SUCCESS${NC}: New key is now active"
else
    echo -e "${RED}FAILED${NC}: New key is not active"
    exit 1
fi

# Clean up
echo "Cleaning up temporary files..."
rm -rf "$TMP_DIR"
echo "Test completed successfully!"
