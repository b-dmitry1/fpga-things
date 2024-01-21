#ifndef CONFIG_H
#define CONFIG_H

#define HEAP_SIZE	256

#define LE16(x) ((x[0] | (x[1] << 8)))
#define LE32(x) ((x[0] | (x[1] << 8u) | (x[2] << 16u) | (x[3] << 24u)))

#endif
