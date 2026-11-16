#!/usr/bin/env bash

find . -type f -name "*.cpp" -print0 | while IFS= read -r -d '' file; do
    if ! grep -qFx '#include <Core/preincluded.hpp>' "$file"; then
        sed -i '1i#include <Core/preincluded.hpp>\n' "$file"
        echo "Updated: $file"
    fi
done