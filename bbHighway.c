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

// Sides to move (colours) -> white = 0, black = 1 -> white = false, black = true
enum { white, black };

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

/******************************************\
===========================================

            Input & Output

===========================================
\******************************************/

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

                Attacks

===========================================
\******************************************/

/*
    not A-file (constant)

  8  0 1 1 1 1 1 1 1
  7  0 1 1 1 1 1 1 1
  6  0 1 1 1 1 1 1 1
  5  0 1 1 1 1 1 1 1
  4  0 1 1 1 1 1 1 1
  3  0 1 1 1 1 1 1 1
  2  0 1 1 1 1 1 1 1
  1  0 1 1 1 1 1 1 1

     a b c d e f g h

    not H-file (constant)

  8  1 1 1 1 1 1 1 0
  7  1 1 1 1 1 1 1 0
  6  1 1 1 1 1 1 1 0
  5  1 1 1 1 1 1 1 0
  4  1 1 1 1 1 1 1 0
  3  1 1 1 1 1 1 1 0
  2  1 1 1 1 1 1 1 0
  1  1 1 1 1 1 1 1 0

     a b c d e f g h

    not HG-file (constant)
    
  8  1 1 1 1 1 1 0 0
  7  1 1 1 1 1 1 0 0
  6  1 1 1 1 1 1 0 0
  5  1 1 1 1 1 1 0 0
  4  1 1 1 1 1 1 0 0
  3  1 1 1 1 1 1 0 0
  2  1 1 1 1 1 1 0 0
  1  1 1 1 1 1 1 0 0

     a b c d e f g h

    not AB-file (constant)

  8  0 0 1 1 1 1 1 1
  7  0 0 1 1 1 1 1 1
  6  0 0 1 1 1 1 1 1
  5  0 0 1 1 1 1 1 1
  4  0 0 1 1 1 1 1 1
  3  0 0 1 1 1 1 1 1
  2  0 0 1 1 1 1 1 1
  1  0 0 1 1 1 1 1 1

     a b c d e f g h
*/

// Not A-file (constant) -> see above for what it looks like! Basically, the a-file is completely cleared.
const U64 not_a_file = 18374403900871474942ULL;

// Not H-file (constant) -> h-file is completely cleared
const U64 not_h_file = 9187201950435737471ULL;

// Not HG-file (constant)
const U64 not_hg_file = 4557430888798830399ULL;

// Not AB-file (constant)
U64 not_ab_file = 18229723555195321596ULL;

// Pawn Attacks Table -> [side][square] -> 2 sides to attack
U64 pawn_attacks[2][64];

// Knight Attacks Table -> [square] -> White knights and black knights have the same possible attacks
U64 knight_attacks[64];

// King Attacks Table -> [square] -> White and Black Kings have the same possible attack patterns
U64 king_attacks[64];

// Generate Pawn Attacks -> what square the pawn is on; what side (colour) it is on
U64 mask_pawn_attacks(int side, int square) {
    // Result Attacks Bitboard
    U64 attacks = 0ULL;
    
    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on the bitboard
    set_bit(bitboard, square);

    // White Pawns
    if (!side) {
        // Checks if the white pawn taking on the top right would mean it looping back around on the a-file (basically stops the top right take when it's on the h-file).
        if ((bitboard >> 7) & not_a_file) { 
            attacks |= (bitboard >> 7);
        }

        // Check for the left file too -> white pawn can't attack the h-file while on the a-file
        if ((bitboard >> 9) & not_h_file) { // bitboard >> 9 = pawn is attacking its top left diagonal.
            attacks |= (bitboard >> 9);
        }
    } else { //Black Pawns
        // Checks if the black pawn taking on the bottom left would mean it looping back around on the h-file (basically stops the bottom left take when it's on the a-file).
        if ((bitboard << 7) & not_h_file) { 
            attacks |= (bitboard << 7);
        }

        // Check for the right file too -> white pawn can't attack the a-file while on the h-file
        if ((bitboard << 9) & not_a_file) { // bitboard << 9 = pawn is attacking its bottom right diagonal.
            attacks |= (bitboard << 9);
        }
    }

    // Return attack map
    return attacks;
}

// Generate Knight Attacks -> Square the knight is on.
U64 mask_knight_attacks(int square) { 
    // Result Attacks Bitboard
    U64 attacks = 0ULL;
    
    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on the bitboard
    set_bit(bitboard, square);

    // Generate knight attacks (15 & 17 for upward or downward motion; need to check a or h files) (10 & 6 for left and right motion upward (flip for downward); need to check ab or hg files)
    // >> is for upward motion (moving left), << is for downward motion (moving right)

    // Up and down attacks
    if ((bitboard >> 15) & not_a_file) { // knight attacking upward on h-file check
        attacks |= (bitboard >> 15);
    }

    if ((bitboard >> 17) & not_h_file) { // knight attacking upward on a-file
        attacks |= (bitboard >> 17);
    }

    if ((bitboard << 15) & not_h_file) { // knight attacking downward on a-file
        attacks |= (bitboard << 15);
    }

    if ((bitboard << 17) & not_a_file) { // knight attacking downward on h-file
        attacks |= (bitboard << 17);
    }

    // Left and right attacks
    if ((bitboard >> 10) & not_hg_file) { // knight attacking upper-left on ab-file check
        attacks |= (bitboard >> 10);
    }

    if ((bitboard >> 6) & not_ab_file) { // knight attacking upper-right on hg-file check
        attacks |= (bitboard >> 6);
    }

    if ((bitboard << 10) & not_ab_file) { // knight attacking lower-right on hg-file check
        attacks |= (bitboard << 10);
    }

    if ((bitboard << 6) & not_hg_file) { // knight attacking lower-left on ab-file check
        attacks |= (bitboard << 6);
    }

    return attacks;
}

U64 mask_king_attacks(int square) { 
    // Result Attacks Bitboard
    U64 attacks = 0ULL;
    
    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on the bitboard
    set_bit(bitboard, square);

    // Generate king attacks (1 for left & right, 8 for up & down, 9 for top-left & bottom-right, 7 for top-right & bottom-left; need to check a or h files for diagonal & horizontal)
    // >> is for upward motion (moving left), << is for downward motion (moving right)

    // Up and down attacks
    attacks |= (bitboard >> 8); // Up
    attacks |= (bitboard << 8); // Down

    // Left and Right attacks
    if ((bitboard >> 1) & not_h_file) { // king attacking left on a-file check
        attacks |= (bitboard >> 1);
    }

    if ((bitboard << 1) & not_a_file) { // king attacking right on h-file
        attacks |= (bitboard << 1);
    }

    // Diagonal Attacks

    if ((bitboard >> 9) & not_h_file) { // king attacking top-left on a-file
        attacks |= (bitboard >> 9);
    }

    if ((bitboard << 9) & not_a_file) { // king attacking bottom-right on h-file
        attacks |= (bitboard << 9);
    }

    if ((bitboard << 7) & not_h_file) { // king attacking bottom-left on a-file
        attacks |= (bitboard << 7);
    }

    if ((bitboard >> 7) & not_a_file) { // king attacking top-right on h-file
        attacks |= (bitboard >> 7);
    }

    return attacks;
}

// Initialize leaper pieces attacks
void init_leapers_attacks() {
    // Loop over 64 board squares
    for (int square = 0; square < 64; square++) {
        // Initialize pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);

        // Initialize knight attacks
        knight_attacks[square] = mask_knight_attacks(square);

        // Initialize king attacks
        king_attacks[square] = mask_king_attacks(square);
    }
}

/******************************************\
===========================================

                Main Driver

===========================================
\******************************************/

int main() {
    // Defined the bitboard
    // U64 bitboard = 0ULL;
    // Initialize leaper attacks
    init_leapers_attacks();
    // Check pawn attacks (look over 64 board squares)
    for (int square = 0; square < 64; square++) {
        // // Initialize pawn attacks
        // print_bitboard(pawn_attacks[black][square]);
        print_bitboard(king_attacks[square]);
    }

    // print_bitboard(mask_pawn_attacks(black, a4));


    // bitboard |= (1ULL << e2);
    // Setting some bits
    // set_bit(bitboard, e4);
    // set_bit(bitboard, e2);
    // set_bit(bitboard, e1);

    // print_bitboard(bitboard);

    // // Reset bit
    // pop_bit(bitboard, e4);

    // print_bitboard(bitboard);

    // pop_bit(bitboard, e4);

    // print_bitboard(bitboard);

    // print human readable board.
    // for (int rank = 8; rank >= 1; rank--) {
    //     printf("\"a%d\", \"b%d\", \"c%d\", \"d%d\", \"e%d\", \"f%d\", \"g%d\", \"h%d,\"\n", rank, rank, rank, rank, rank, rank, rank, rank);
    // }

    return 0;
}
