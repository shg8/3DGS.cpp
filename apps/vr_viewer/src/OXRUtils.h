#ifndef OXRUTILS_H
#define OXRUTILS_H

/**
    A portion of the code in this file is modified from xrgears, which is licensed as follows:

    The MIT License (MIT)

    Copyright 2019 Collabora Ltd.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

**/

#include <cstdint>
#include <string>
#include <vector>
#include <openxr/openxr.h>

namespace OXR {
    inline bool isExtensionSupported(const std::string &extensionName) {
        uint32_t extensionCount;
        XrResult result = xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
        if (XR_FAILED(result)) {
            return false;
        }

        std::vector<XrExtensionProperties> extensionProperties(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
        result = xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount,
                                                        extensionProperties.data());
        if (XR_FAILED(result)) {
            return false;
        }

        for (const auto &extensionProperty: extensionProperties) {
            if (extensionName == extensionProperty.extensionName) {
                return true;
            }
        }

        return false;
    }

    class CharArrayWrapper {
    public:
        explicit CharArrayWrapper(const std::vector<std::string> &vec) {
            charArray.resize(vec.size());

            for (size_t i = 0; i < vec.size(); ++i) {
                charArray[i] = vec[i].c_str();
            }
        }

        [[nodiscard]] const char *const*get() const {
            return charArray.data();
        }

    private:
        std::vector<const char *> charArray;
    };
}

#endif //OXRUTILS_H
