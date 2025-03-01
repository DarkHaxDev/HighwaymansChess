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
#include <string.h>
#include <locale.h>

#ifdef _WIN64
    #include <windows.h>
#endif

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
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

// Count bits -> using popcount to take advantage of modern hardware (thanks pedrogodinho4649)
#define count_bits(bitboard) __builtin_popcountll(bitboard)
// Find the least significant 1st bit index -> as suggested by yashshingade1593
#define get_ls1b_index(bitboard) (__builtin_ffsll(bitboard) - 1)

// Other way to count bits (Brian Kernighan's way)
static inline int BK_count_bits(U64 bitboard) {
    // bit counter
    int count = 0;

    // Consecutively reset least significant 1st bits in bitboard (basically the topmost bits in the board representation are continuously set to 0)
    while (bitboard) { // While bitboard isn't 0
        // increment count
        count++;

        // Reset least significant first bit
        bitboard &= bitboard - 1;
    }

    return count;
}

// Other way to get the least significant 1st bit index (LS1B)
static inline int CMK_get_ls1b_index(U64 bitboard) {
    // Make sure that the bitboard isn't 0
    if (bitboard) {
        // Count trailing bits before LS1B
        return BK_count_bits((bitboard & ~bitboard + 1) - 1);
    } else {
        // Return illegal index
        return -1;
    }
}

/******************************************\
===========================================

        Bitboards Stuff

===========================================
\******************************************/

// Enumerate (enum) the bitboard squares
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
    no_sq
};

// Sides to move (colours) -> white = 0, black = 1 -> white = false, black = true & also there's both
enum { white, black, both };

// bishop and rook
enum { rook, bishop };

// Encode pieces
/// Uppercase for white pieces; lowercase for black pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };


const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};


// define piece bitboards
U64 bitboards[12]; // 6 bitboards for each piece on each side

// Occupancy bitboards
U64 occupancies[3]; // White, black, and both sides occupancies

// side to move
int side;

// enpassant square
int enpassant = no_sq;

// castling rights (4 bit flag)
/*  Castling Bits Binary Representation

    bin     dec
    0001    1       white king can castle to the king side
    0010    2       white king can castle to the queen side
    0100    4       black king can castle to the king side
    1000    8       black king can castle to the queen side

    ex.
    1111        both sides can castle in both directions
    1001        black king can castle to queen side; white king can castle to king side

*/
/// Castling enumerations
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

int castle;

// ASCII pieces
/// Can be indexed by the piece enumeration (see above)
char ascii_pieces[12] = "PNBRQKpnbrqk";

// Unicode Pieces
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", 
                            "♟", "♞", "♝", "♜", "♛", "♚"};

// Convert ASCII character to encoded unicode constants
int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k,
};

void enable_unicode_support() {
    #ifdef _WIN64
        // set the console output to UTF-8
        SetConsoleOutputCP(CP_UTF8);
    #else
        // set the locale to system default for Mac + Linux
        setlocale(LC_ALL, "");
    #endif
}

/******************************************\
===========================================

        Random Number Generation

===========================================
\******************************************/

// Pseudo-random number state -> Got it from Chess Programming just so that I can follow along with his tutorial.
// Will consider generating my own seed state later
unsigned int random_state = 1804289383;

// generate 32-bit pseudo-legal numbers
unsigned int get_random_U32_number() {
    // get current state
    unsigned int number = random_state;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // Update random number state
    random_state = number;

    // return random number
    return number;
}

// Generate 64-bit pseudo-legal numbers

U64 get_random_U64_number() {
    // define 4 random U32 numbers
    U64 n1, n2, n3, n4;

    // initialize random numbers & excise the 16 bits from the MS1b side. Essentially leaves 4 numbers of 16 bits in size.
    // Slices and grabs upper (relative to most significant first bit side -> MS1B side) 16 bits
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;

    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// Generate magic number candidate
U64 generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}



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

// Print the board
void print_board() {
    // Print offset for prettification
    printf("\n");
    // Loop over the board ranks
    for (int rank = 0; rank < 8; rank++) {
        printf("  %d ", 8 - rank);
        for (int file = 0; file < 8; file++) {
            // Get the current square
            int square = rank * 8 + file;

            // Define the piece variable
            int piece = -1;

            // Loop over all piece bitboards
            for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                if (get_bit(bitboards[bb_piece], square)) { // If the current piece type and colour exists on the current square
                    piece = bb_piece;
                }
            }
            printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]); // "." refers to the string representation of the character '.'
        }
        // Print a new line every rank
        printf("\n");
    }
    
    // print board files
    printf("\n     a b c d e f g h \n\n");

    // print side to move
    printf("    Side:      %s\n", !side ? "White" : "Black");

    // Print the enpassant square
    printf("    Enpass:    %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "Not Available");

    // Print castling rights
    printf("    Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
                                           (castle & wq) ? 'Q' : '-',
                                           (castle & bk) ? 'k' : '-',
                                           (castle & bq) ? 'q' : '-');
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
const U64 not_ab_file = 18229723555195321596ULL;

// Relevant Occupancy Bit Count for every square on board -> how many squares a given piece can reach from its position on the board (edges not included)
// Another way to think about it is how many pieces can block a given piece from reaching the edges of the board.
const int bishop_relevant_occ_bits[64] = {
    6,  5,  5,  5,  5,  5,  5,  6, 
    5,  5,  5,  5,  5,  5,  5,  5,
    5,  5,  7,  7,  7,  7,  5,  5,
    5,  5,  7,  9,  9,  7,  5,  5,
    5,  5,  7,  9,  9,  7,  5,  5,
    5,  5,  7,  7,  7,  7,  5,  5,
    5,  5,  5,  5,  5,  5,  5,  5,
    6,  5,  5,  5,  5,  5,  5,  6,
};

const int rook_relevant_occ_bits[64] = {
    12,  11,  11,  11,  11,  11,  11,  12, 
    11,  10,  10,  10,  10,  10,  10,  11,
    11,  10,  10,  10,  10,  10,  10,  11,
    11,  10,  10,  10,  10,  10,  10,  11,
    11,  10,  10,  10,  10,  10,  10,  11,
    11,  10,  10,  10,  10,  10,  10,  11,
    11,  10,  10,  10,  10,  10,  10,  11,
    12,  11,  11,  11,  11,  11,  11,  12,
};

U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL,
};
U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL,
};

// Pawn Attacks Table -> [side][square] -> 2 sides to attack
U64 pawn_attacks[2][64];

// Knight Attacks Table -> [square] -> White knights and black knights have the same possible attacks
U64 knight_attacks[64];

// King Attacks Table -> [square] -> White and Black Kings have the same possible attack patterns
U64 king_attacks[64];

// Bishop Attacks masks
U64 bishop_masks[64];

// Rook attack masks
U64 rook_masks[64];

// Bishop Attacks Table -> [square][occupancies] -> occupancies represent number of possible occupancies (blocking an attack). For Bishops, it's 512.
U64 bishop_attacks[64][512];

// Rook Attacks Table -> [square][occupancies]
U64 rook_attacks[64][4096];

// Slider attacks table -> includes bishop & rook attacks; indexable using offset.
//// Total number of occupancy boards (divined by summing them up; see below) is 107648, which will be the size of our fancy slider attack look up table.
//// This is compared to the plain method (see above) which has bishop and rook attacks tables, which, in total, uses 294912 bitboards (which aren't all necessary).
/*
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum = sum + (int)(1 << bishop_relevant_occ_bits[i]);
        sum = sum + (int)(1 << rook_relevant_occ_bits[i]);
    }
    printf("Total occupancy boards: %d", sum);
*/
U64 slider_attacks[107648];
int rook_offset[64];
int bishop_offset[64];


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


// Mask Bishop Attacks -> All the pieces that a bishop could occupy if it moves (assuming that all squares are not occupied), not including the edges of the board.
U64 mask_bishop_attacks(int square) {
    // Result Attacks Bitboard
    U64 attacks = 0ULL;

    // initialize rank & files

    int r, f;

    // initialize target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // mask relevant bishop occupany bits -> squares that the bishop can slide to (not the edges!)
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) { // Bottom-right sliding
        attacks |= (1ULL << (r * 8 + f));
    }

    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) { // Top-right sliding
        attacks |= (1ULL << (r * 8 + f));
    }

    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) { // Bottom-left sliding
        attacks |= (1ULL << (r * 8 + f));
    }

    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) { // Top-left sliding
        attacks |= (1ULL << (r * 8 + f));
    }


    // Return attack map
    return attacks;
}

// Mask rook attacks -> All the pieces that a bishop could occupy if it moves (assuming that all squares are not occupied), not including the edges of the board.
U64 mask_rook_attacks(int square) {
    // Result Attacks Bitboard
    U64 attacks = 0ULL;

    // initialize rank & files

    int r, f;

    // initialize target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // mask relevant rook occupany bits -> squares that the rook can slide to (not the edges!)
    for (r = tr + 1; r <= 6; r++) { // sliding downward
        attacks |= (1ULL << (r * 8 + tf));
    }

    for (r = tr - 1; r >= 1; r--) { // sliding upward
        attacks |= (1ULL << (r * 8 + tf));
    }

    for (f = tf + 1; f <= 6; f++) { // sliding to the right
        attacks |= (1ULL << (tr * 8 + f));
    }

    for (f = tf - 1; f >= 1; f--) { // sliding to the left
        attacks |= (1ULL << (tr * 8 + f));
    }


    // Return attack map
    return attacks;
}

// Generate Bishop attacks on the fly -> Basically, where a bishop can really move to given an occupancy board (or rather, a bitboard that has other pieces on it).
//// Once its attack ray impacts another piece (represented by a bit of 1), it stops right there. Includes the edges of the board.
U64 bishop_attacks_on_the_fly(int square, U64 blockboard) {
    // Result Attacks Bitboard
    U64 attacks = 0ULL;

    // initialize rank & files

    int r, f;

    // initialize target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // generate bishop attacks
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) { // Bottom-right sliding
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockboard) {
            break;
        }
    }

    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) { // Top-right sliding
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockboard) {
            break;
        }
    }

    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) { // Bottom-left sliding
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockboard) {
            break;
        }
    }

    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) { // Top-left sliding
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockboard) {
            break;
        }
    }


    // Return attack map
    return attacks;
}

// Generate rook attacks on the fly -> Where a rook can really move to given an occupancy board. See the above for a more detailed explanation.
U64 rook_attacks_on_the_fly(int square, U64 blockboard) {
    // Result Attacks Bitboard
    U64 attacks = 0ULL;

    // initialize rank & files

    int r, f;

    // initialize target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // generate rook attacks
    for (r = tr + 1; r <= 7; r++) { // sliding downward
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & blockboard) {
            break;
        }
    }

    for (r = tr - 1; r >= 0; r--) { // sliding upward
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & blockboard) {
            break;
        }
    }

    for (f = tf + 1; f <= 7; f++) { // sliding to the right
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & blockboard) {
            break;
        }
    }

    for (f = tf - 1; f >= 0; f--) { // sliding to the left
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & blockboard) {
            break;
        }
    }


    // Return attack map
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

// Set Occupancies -> index, number of bits in the attack mask, and the attack mask itself
// Basically, the occupancy bitboard holds all possible positions for the given attack mask to be blocked!
// In other words, a 1 on the board simply means that there's a piece there (that will block the attack mask's piece from reaching the edges of the board)
// To put it fancily, this calculates the enumeration of all possible blocker configurations for sliding pieces!
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    //// Index is an integer that (when looked at as a binary number) specifies which squares in the attack_mask should be considered as presently occupied.
    //// Remember, the attack_mask is the range of all possible attacks (not including edges) for a given piece.
    //// This function pretty much wipes the attack_mask clean except for the pieces that block the piece
    // Initialize occupancy board
    U64 occupancy = 0ULL;

    // Loop over the range of bits within the attack mask
    for (int count = 0; count < bits_in_mask; count++) {
        // Get LS1B index of attack mask
        int square = get_ls1b_index(attack_mask);

        // Pop the LS1B in the attack map
        pop_bit(attack_mask, square);

        // Make sure that occupancy is on the board
        //// If the square that was wiped from the attack_mask is supposed to be occupied (according to the bits specified by the index), then add it to the occupancy board
        if (index & (1 << count)) {
            // Populate occupancy map
            set_bit(occupancy, square);
        }
    }

    // return occupancy map
    return occupancy;
}

// Get our bishop attacks - Fancy
static inline U64 get_bishop_attacks(int square, U64 occupancy) {
    // Get bishop attacks assuming current board occupancy
    int offset = bishop_offset[square];
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_occ_bits[square];
    return slider_attacks[offset + occupancy];
}

static inline U64 get_rook_attacks(int square, U64 occupancy) {
    // Get rook attacks assuming current board occupancy
    int offset = rook_offset[square];
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_occ_bits[square];
    return slider_attacks[offset + occupancy];
}

// Get our bishop attacks - Plain

/******************************************\
===========================================

                Magic

===========================================
\******************************************/




// Find the appropriate magic number
U64 find_magic_number(int square, int rel_occ_bits, int bishop_flag) {

    //// Initialize everything!!! //

    // Initialize occupancies
    U64 occupancies[4096];

    // Initialize attack tables
    U64 attacks[4096]; 

    // Initialize used attacks
    U64 used_attacks[4096];

    // Initialize attack mask for a current piece
    U64 attack_mask = bishop_flag ? mask_bishop_attacks(square) : mask_rook_attacks(square);

    // Initialize Occupancy Indices
    int occupancy_indices = 1 << rel_occ_bits;

    // Loop over occupancy indices -> Generate all possible occupancy boards for a given attack mask
    for (int index = 0; index < occupancy_indices; index++) {
        // Generate the current occupancy board
        //// By increasing the index, it covers all possible occupancies around the piece. Remember: index is a binary number whose bits inform which possible occupancies are actually occupied (for this occupancy board)
        occupancies[index] = set_occupancy(index, rel_occ_bits, attack_mask);

        // Generate the possible attacks for the bishop/rook given the current occupancy board
        //// Different occupancy boards can have the same possible attacks. Pieces that block an attack by a rook for example might have another piece behind them. However, even if that other piece isn't present in another
        //// occupancy board, it won't affect how the blocking piece blocks the rook's attack, thus resulting in the same possible attack board.
        attacks[index] = bishop_flag ? bishop_attacks_on_the_fly(square, occupancies[index]) : rook_attacks_on_the_fly(square, occupancies[index]);
    }

    // Test magic numbers loop -> really high number to try and make sure that each occupancy index has their corresponding magic number
    for (int random_count = 0; random_count < 100000000; random_count++) {
        // Generate magic number candidate
        U64 magic_number = generate_magic_number();

        // If inappropriate magic number (I'm not sure what constitutes as inappropriate), then skip it
        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) {
            continue;
        }

        // Initialize used attacks to 0
        memset(used_attacks, 0ULL, sizeof(used_attacks));

        // Initialize index & fail flag (for failed magic number, which return a 0 for it instead of the number)
        int index, fail;

        // test magic index loop -> looks over all possible occupancy indices (occupancy boards)
        for (index = 0, fail = 0; !fail && index < occupancy_indices; index++) {
            // Initialize magic index
            //// occupancies (all possible occupancy boards for the given square & piece).
            //// occupancies[index] = current occupancy board -> U64 bitboard
            //// Essentially, we multiply the magic number by the occupancy bitboard to spread bits across the 64-bit register in a pseudo-random way
            // -> The right shift gets rid of irrelevant occupancy bits (keeps the index in the range of 0 to 2^(rel_occ_bits) - 1), keeping the topmost rel_occ_bits. rel_occ_bits is the number of bits required to index the attack table.
            // Remember, for a square that has 4096 possible occupancy boards (associated with 4096 possible attacks), we need an index that could get us to the associated attack for each.
            // This means we would need an index between 0 to 4095 (0 to 2^(rel_occ_bits) - 1), or rather, a 2^12 integer. By right shifting the amount of (64 - 12), we would get rid of all the other bits and only keep 12 bits for the index.
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - rel_occ_bits));

            //// Essentially, what the below does is ensure that every magic number is either:
            //// 1. Pointing to a full attack mask (given the current occupancy board) that only the magic_index points to
            //// 2. Pointing to a full attack mask that was already previously pointed to by another magic_index, but also is valid given the current occupancy board
            ///// (2 means that another attack mask exists there, but it'd still work as the proper possible attacks given the current occupancy board, so it's fine.)
            // If the magic index works and the index isn't used
            if (used_attacks[magic_index] == 0ULL) { // If the magic_index isn't pointing to a full attack mask (given the current occupancy board), then do so.
                // Set used_attacks at the index to the attacks with the same index
                used_attacks[magic_index] = attacks[index];
            } else if (used_attacks[magic_index] != attacks[index]) { // If the magic index points to the incorrect possible attacks given the current occupancy board, then the magic number doesn't work.
                // Magic index doesn't work
                fail = 1;
            }
            // The above ensures that magic numbers will always generate magic indices that will never access an attack mask that isn't suitable for its associated occupancy board.
            /// Multiple occupancy boards can access the same attack mask without issue, provided its one that's possible given the occupancies.

        }
        // If magic number works, return it
        if (!fail) {
            return magic_number;
        }
    }
    // If magic number doesn't work
    printf("    Magic number fails!");
    return 0ULL;
}

// Initialize Magic Numbers -> for testing purposes
void initialize_magic_numbers() {
    // Loop over 64 squares
    for (int square = 0; square < 64; square++) {
        // Initialize rook magic numbers
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_occ_bits[square], rook);
    }

    // printf("\n\n");

    for (int square = 0; square < 64; square++) {
        // Initialize bishop magic numbers
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_occ_bits[square], bishop);
    }

    // for (int square = 0; square < 64; square++) {
    //     printf(" 0x%llxULL,\n", rook_magic_numbers[square]);
    // }

    // printf("\n\n");

    // for (int square = 0; square < 64; square++) {
    //     printf(" 0x%llxULL,\n", bishop_magic_numbers[square]);
    // }
}

// Initialize slider piece's attack tables using Fancy Magic Bitboards'


int rook_start_offset;
// // Returns the offset necessary for rook attack tables to begin
void init_slider_attacks(int bishop_flag) {
    // Loop over all 64 squares
    int table_offset = bishop_flag ? 0 : rook_start_offset;
    
    for (int square = 0; square < 64; square++) {
        // Initialize the bishop & rook masks
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        // Get the attack mask for the current square & piece type
        U64 attack_mask = bishop_flag ? bishop_masks[square] : rook_masks[square];

        // Grab relevant occupancy bit count
        int rel_occ_bits = count_bits(attack_mask);

        // Initialize occupancy indices
        //// Occupancy indices is essentially the total number of combinations in which a piece can block the attacking rook/bishop from that square
        //// An occupancy index is an integer that specifies which squares within a given attack mask are currently occupied (from least to most significant bits)
        //// Essentially, if an attack mask had three possible occupying bits/pieces (or occupied squares), then occupancy_indices will total 2^3 - 1 = 7. Each occupying index would represent a 3 bit number,
        //// Where the bits specify which squares are occupied (for the purposes of generating combinations of occupancy boards).
        int occupancy_indices = (1 << rel_occ_bits);
        // printf("%d \n", occupancy_indices);
        // print(occupancy_indices)
        
        // Loop over occupancy indices -> Run through all possible occupancy board combinations for the given attack mask (all possible ways an attack mask could be occupied).
        for (int index = 0; index < occupancy_indices; index++) {
            // Initialize current occupancy variation
            U64 occupancy = set_occupancy(index, rel_occ_bits, attack_mask);
            // Bishop
            if (bishop_flag) {
                // Grab our magic index using the magic numbers
                int magic_index = (int)((occupancy * bishop_magic_numbers[square]) >> (64 - rel_occ_bits));

                // Use magic index to get our bishop attacks
                slider_attacks[table_offset + magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            } else { // Rook
                // Grab our magic index using the magic numbers
                int magic_index = (int)((occupancy * rook_magic_numbers[square]) >> (64 - rel_occ_bits));

                // Use magic index to get our rook attacks
                slider_attacks[table_offset + magic_index] = rook_attacks_on_the_fly(square, occupancy);
            }
        }
        
        if (bishop_flag) { // Record the proper offset for each square in the appropriate tables
            bishop_offset[square] = table_offset;
        } else {
            rook_offset[square] = table_offset;
        }
        table_offset = table_offset + occupancy_indices; // Update the offsets
        
    }
    // printf("hello world");
    if (bishop_flag) { // Sets proper offset for rook attacks to begin -> only useful for initial move generation
        rook_start_offset = table_offset;
    }
}

// Initialize slider piece's attack tables
void init_slider_attacks_plain(int bishop_flag) {
    // Loop over 64 squares
    for (int square = 0; square < 64; square++) {
        // initialize bishop & rook masks
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        // initialize current mask
        U64 attack_mask = bishop_flag ? bishop_masks[square] : rook_masks[square];

        // Grab relevant occupancy bit count
        int rel_occ_bits = count_bits(attack_mask);

        // Initialize occupancy indices
        int occupancy_indices = (1 << rel_occ_bits);

        // Loop over occupancy indices
        for (int index = 0; index < occupancy_indices; index++) {
            // Initialize current occupancy variation
            U64 occupancy = set_occupancy(index, rel_occ_bits, attack_mask);
            // Bishop
            if (bishop_flag) {
                // Grab our magic index using the magic numbers
                int magic_index = (int)((occupancy * bishop_magic_numbers[square]) >> (64 - rel_occ_bits));

                // Use magic index to get our bishop attacks
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            } else { // Rook
                // Grab our magic index using the magic numbers
                int magic_index = (int)((occupancy * rook_magic_numbers[square]) >> (64 - rel_occ_bits));

                // Use magic index to get our rook attacks
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
            }
        }
    }
    
}

/******************************************\
===========================================

            Initialize All

===========================================
\******************************************/

void initialize_all() {
    // initialize unicode stuff
    enable_unicode_support();

    // initialize leaper pieces atacks
    init_leapers_attacks();
    

    // initialize magic numbers
    // initialize_magic_numbers();

    // Initialize slider attacks
    init_slider_attacks(bishop);
    // printf("hello world");
    init_slider_attacks(rook);
    // printf("hello world");
}

/******************************************\
===========================================

                Main Driver

===========================================
\******************************************/

int main() {
    // Initialize everything
    initialize_all();

    // TO DO: Figure out how many occupancy boards exist (for both rook + bishop pieces) so that I can use it to set up the correct memory for the unified attack table.
    
    // Set white pieces
    set_bit(bitboards[P], a2);
    set_bit(bitboards[P], b2);
    set_bit(bitboards[P], c2);
    set_bit(bitboards[P], d2);
    set_bit(bitboards[P], e2);
    set_bit(bitboards[P], f2);
    set_bit(bitboards[P], g2);
    set_bit(bitboards[P], h2);

    set_bit(bitboards[R], a1);
    set_bit(bitboards[R], h1);

    set_bit(bitboards[N], b1);
    set_bit(bitboards[N], g1);

    set_bit(bitboards[B], c1);
    set_bit(bitboards[B], f1);

    set_bit(bitboards[Q], d1);
    set_bit(bitboards[K], e1);

    // Set black pieces
    set_bit(bitboards[p], a7);
    set_bit(bitboards[p], b7);
    set_bit(bitboards[p], c7);
    set_bit(bitboards[p], d7);
    set_bit(bitboards[p], e7);
    set_bit(bitboards[p], f7);
    set_bit(bitboards[p], g7);
    set_bit(bitboards[p], h7);

    set_bit(bitboards[r], a8);
    set_bit(bitboards[r], h8);

    set_bit(bitboards[n], b8);
    set_bit(bitboards[n], g8);

    set_bit(bitboards[b], c8);
    set_bit(bitboards[b], f8);

    set_bit(bitboards[q], d8);
    set_bit(bitboards[k], e8);

    // print white pawn bitboard
    print_bitboard(bitboards[P]);

    // print piece
    // printf("piece: %c\n", ascii_pieces[P]);

    // // print unicode piece
    // printf("piece: %s\n", unicode_pieces[P]);

    // printf("piece: %c\n", ascii_pieces[char_pieces['K']]);
    // printf("piece: %s\n", unicode_pieces[char_pieces['K']]);

    // Print chess board
    side = black;
    enpassant = e3;
    castle |= wk;
    castle |= wq;
    castle |= bk;
    castle |= bq;
    print_board();

    // print all bitboards
    for (int piece = P; piece <= k; piece++) {
        // print current piece bitboard
        print_bitboard(bitboards[piece]);
    }

    // define test bitboard
    // U64 occupancy = 0ULL;
    // print_bitboard(occupancy);

    // // print bishop attacks
    // set_bit(occupancy, c5);
    // set_bit(occupancy, f2);
    // set_bit(occupancy, g7);
    // set_bit(occupancy, b2);
    // set_bit(occupancy, g5);
    // set_bit(occupancy, e2);
    // set_bit(occupancy, e7);

    // print_bitboard(get_rook_attacks(e5, occupancy));

    // print_bitboard(get_bishop_attacks(d4, occupancy));

    return 0;
}
