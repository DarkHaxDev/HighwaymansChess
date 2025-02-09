/******************************************\
===========================================

            Highway Chess
        Bitboard Chess Engine

        by DarkHaxDev

    Following the video series by Chess Programming!

    To build and run, use the following command: mingw32-make debug && bbHighway.exe
    -> The debug part is optional. Remove it if you don't want to debug: mingw32-make && bbHighway.exe

===========================================
\******************************************/

// System Headers

#include <stdio.h>

// Define bitboard data type

#define U64 unsigned long long

/******************************************\
===========================================

            Bit Manipulations

===========================================
\******************************************/

// Set + get + pop Macros

// Is the bit available (0, I think) at the target square?
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
// Set the bit at the target square to 1 (I think)
#define set_bit(bitboard, square) (bitboard |= (1ULL) << square)
// Set the bit at the target square at 0
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

// Enumerate (enum) the bitboard squares
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

/*
FOR FUTURE USE:

"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8,"
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7,"
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6,"
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5,"
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4,"
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3,"
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2,"
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1,"

*/

// Print Bitboard
void print_bitboard(U64 bitboard) {
    printf("\n");

    // Loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        // loop over board files
        for (int file = 0; file < 8; file++) {
            // Convert file & rank into square index
            int square = rank * 8 + file;

            // Print ranks
            if (!file){
                printf("  %d ", 8 - rank);
            }

            // Print bit states for whole board (either 1 or 0)
            printf(" %d", get_bit(bitboard, square) ? 1 : 0);
        }
        // print new line every rank that's printed
        printf("\n");
    }
    // Print board files
    printf("\n     a b c d e f g h \n\n");

    // Print the bitboard as an unsigned decimal number
    printf("     Bitboard: %llud\n\n", bitboard);
}


/******************************************\
===========================================

                Main Driver

===========================================
\******************************************/

int main() {
    // Defined the bitboard
    U64 bitboard = 0ULL;

    // bitboard |= (1ULL << e2);
    // Setting some bits
    set_bit(bitboard, e4);
    set_bit(bitboard, e2);
    set_bit(bitboard, e1);

    print_bitboard(bitboard);

    // Reset bit
    pop_bit(bitboard, e4);

    print_bitboard(bitboard);

    pop_bit(bitboard, e4);

    print_bitboard(bitboard);

    // print human readable board.
    // for (int rank = 8; rank >= 1; rank--) {
    //     printf("\"a%d\", \"b%d\", \"c%d\", \"d%d\", \"e%d\", \"f%d\", \"g%d\", \"h%d,\"\n", rank, rank, rank, rank, rank, rank, rank, rank);
    // }

    return 0;
}
