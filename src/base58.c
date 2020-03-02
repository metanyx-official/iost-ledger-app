#include "base58.h"
#include <os.h>
#include <cx.h>

/** array of base58 aplhabet letters */
static const char BASE_58_ALPHABET[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', /**/ 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(
    const char* alphabet,
    const unsigned int alphabet_len,
    const void* in,
    const unsigned int in_length,
    unsigned char* out
) {
    char tmp[64] = {};
    char buffer[128] = {};
    unsigned char buffer_ix = 0;
    unsigned char startAt = 0;
    unsigned char zeroCount = 0;
    unsigned int true_out_length = 0;

    if (in_length <= sizeof(tmp)) {
        os_memmove(tmp, in, in_length);
        while ((zeroCount < in_length) && (tmp[zeroCount] == 0)) {
            ++zeroCount;
        }
        buffer_ix = 2 * in_length;
        if (buffer_ix <= sizeof(buffer)) {
            startAt = zeroCount;
            while (startAt < in_length) {
                unsigned short remainder = 0;
                unsigned char divLoop;
                for (divLoop = startAt; divLoop < in_length; divLoop++) {
                    unsigned short digit256 = (unsigned short) (tmp[divLoop] & 0xff);
                    unsigned short tmpDiv = remainder * 256 + digit256;
                    tmp[divLoop] = (unsigned char) (tmpDiv / alphabet_len);
                    remainder = (tmpDiv % alphabet_len);
                }
                if (tmp[startAt] == 0) {
                    ++startAt;
                }
                buffer[--buffer_ix] = *(alphabet + remainder);
            }
            while ((buffer_ix < (2 * in_length)) && (buffer[buffer_ix] == *(alphabet + 0))) {
                ++buffer_ix;
            }
            while (zeroCount-- > 0) {
                buffer[--buffer_ix] = *(alphabet + 0);
            }
            true_out_length = (2 * in_length) - buffer_ix;
            os_memmove(out, (buffer + buffer_ix), true_out_length);
        }
    }
    return true_out_length;
}

/** encodes in_length bytes from in into base-58, writes the converted bytes to out, stopping when it converts out_length bytes.  */
unsigned int encode_base_58(
    const void* const in,
    const unsigned int in_len,
    unsigned char* out
) {
    return encode_base_x(
        BASE_58_ALPHABET,
        sizeof(BASE_58_ALPHABET),
        in,
        in_len,
        out
    );
}
