#!/bin/bash
# Script to update the wasmbuild.sh file to include MPFR

# Set variables
WASMBUILD_FILE="/volumes/bigcode/hydra_sdk/wasmbuild.sh"

# Create a backup of the original file
cp "${WASMBUILD_FILE}" "${WASMBUILD_FILE}.bak"

# Update the CXX_FLAGS to include MPFR
sed -i '' 's/-I${GMP_INCLUDE_DIR} -I${BOTAN_INCLUDE_DIR} -I${JSON_INCLUDE_DIR}/-I${GMP_INCLUDE_DIR} -I${MPFR_INCLUDE_DIR} -I${BOTAN_INCLUDE_DIR} -I${JSON_INCLUDE_DIR}/g' "${WASMBUILD_FILE}"

# Update the first emcmake command (verbose mode)
sed -i '' '/if \[ \$VERBOSE -eq 1 \]; then/,/else/s/-DGMP_LIBRARY="\${GMP_LIBRARY}" \\/-DGMP_LIBRARY="\${GMP_LIBRARY}" \\\n      -DMPFR_INCLUDE_DIR="\${MPFR_INCLUDE_DIR}" \\\n      -DMPFR_LIBRARY="\${MPFR_LIBRARY}" \\/g' "${WASMBUILD_FILE}"

# Update the second emcmake command (non-verbose mode)
sed -i '' '/else/,/fi/s/-DGMP_LIBRARY="\${GMP_LIBRARY}" \\/-DGMP_LIBRARY="\${GMP_LIBRARY}" \\\n      -DMPFR_INCLUDE_DIR="\${MPFR_INCLUDE_DIR}" \\\n      -DMPFR_LIBRARY="\${MPFR_LIBRARY}" \\/g' "${WASMBUILD_FILE}"

echo "wasmbuild.sh has been updated to include MPFR!"
