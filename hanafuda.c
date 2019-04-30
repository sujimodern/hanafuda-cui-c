#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hanafuda.h"

enum Size { empty = 0, num_card_char = 2, num_per_month = 4, hand = 8, num_months = 12, half = 24, full = 48, buffer_max = 256 };
enum Category { kasufuda, tanzaku, tanefuda, ao, aka, inoshishi, shika, cho, tsuki, maku,
                sakazuki, ame, hikari, category_size, };
enum Meld { goko, shiko, ameshiko, sanko, hanami, tsukimi, inoshikacho, inoshikacho_addon, akatan, akatan_addon,
            aotan, aotan_addon, tane, tane_addon, tan, tan_addon, kasu, kasu_addon, meld_size, };
enum Player { banker_player, punter_player, player_size, };

struct hanafuda {
  size_t local_index;
  int month_index;
  int category;
};

char const kMeldLabel[meld_size][20] = { 
  "五光　　　", "四光　　　", "雨四光　　", "三光　　　", "花見で一杯", "月見で一杯", "猪鹿蝶　　", "猪鹿蝶ほか", "赤短　　　", "赤短ほか",
  "青短　　　", "青短ほか　", "タネ　　　", "タネほか　", "タン　　　", "タンほか　", "カス　　　", "カスほか　",
};

char const kCardChar[num_card_char][num_months][num_per_month][4] = {
  { 
    {"松", "松", "松", "松"},
    {"梅", "梅", "梅", "梅"},
    {"桜", "桜", "桜", "桜"},
    {"藤", "藤", "藤", "藤"},
    {"菖", "菖", "菖", "菖"},
    {"牡", "牡", "牡", "牡"},
    {"萩", "萩", "萩", "萩"},
    {"芒", "芒", "芒", "芒"},
    {"菊", "菊", "菊", "菊"},
    {"紅", "紅", "紅", "紅"},
    {"柳", "柳", "柳", "鬼"},
    {"桐", "桐", "桐", "桐"},
  }, {
    {"鶴", "赤", "　", "　"},
    {"鶯", "赤", "　", "　"},
    {"幕", "赤", "　", "　"},
    {"鳥", "短", "　", "　"},
    {"橋", "青", "　", "　"},
    {"蝶", "青", "　", "　"},
    {"猪", "短", "　", "　"},
    {"月", "雁", "　", "　"},
    {"盃", "青", "　", "　"},
    {"鹿", "青", "　", "　"},
    {"小", "燕", "短", "鼓"},
    {"鳳", "　", "　", "　"},
  }
};

int const kCategoryTable[num_months][num_per_month] = {
  {hikari, aka, kasufuda, kasufuda},
  {tanefuda, aka, kasufuda, kasufuda},
  {maku, aka, kasufuda, kasufuda},
  {tanefuda, tanzaku, kasufuda, kasufuda},
  {tanefuda, tanzaku, kasufuda, kasufuda},
  {cho, ao, kasufuda, kasufuda},
  {inoshishi, tanzaku, kasufuda, kasufuda},
  {tsuki, tanefuda, kasufuda, kasufuda},
  {sakazuki, ao, kasufuda, kasufuda},
  {shika, ao, kasufuda, kasufuda},
  {ame, tanefuda, tanzaku, kasufuda},
  {hikari, kasufuda, kasufuda, kasufuda},
};

void SwapCards(size_t len_deck, hanafuda deck[len_deck], size_t i, size_t j) {
  hanafuda t = deck[i];
  deck[i] = deck[j];
  deck[j] = t;
}

void ShuffleDeck(size_t len_deck, hanafuda deck[len_deck], size_t times) {
  srand(time(0));
  for (size_t c = 0; c < times; ++c) {
    size_t i = rand() % len_deck;
    size_t j = rand() % len_deck;
    SwapCards(len_deck, deck, i, j);
  } 
}

void DumpCards(size_t len, hanafuda fuda[len]) {
  for (size_t i = 0; i < len; ++i) {
    printf("i: %lu, month_index:%d, local_index:%lu category:%d\n", i, fuda[i].month_index, fuda[i].local_index, fuda[i].category);
  }
}

void InitDeck(size_t len_deck, hanafuda deck[len_deck]) {
  size_t serial_index = 0;
  for (size_t m = 0; m < num_months; ++m) {
    for (size_t i = 0; i < num_per_month; ++i) {
      serial_index = m * num_per_month + i;
      deck[serial_index].local_index = i;
      deck[serial_index].month_index = m;
      deck[serial_index].category = kCategoryTable[m][i];
    }
  }
}

void Deal(size_t len_deck, hanafuda deck[len_deck],
          size_t len_banker_hand, hanafuda banker_hand[len_banker_hand],
          size_t len_punter_hand, hanafuda punter_hand[len_punter_hand],
          size_t len_layout, hanafuda layout[len_layout],
          size_t len_stock, hanafuda stock[len_stock]) {
  size_t start = 0;
  for (size_t i = 0; i < len_banker_hand; ++i) {
    banker_hand[i] = deck[start+i];
  }
  start += len_banker_hand;
  for (size_t i = 0; i < len_punter_hand; ++i) {
    punter_hand[i] = deck[start+i];
  }
  start += len_punter_hand;
  for (size_t i = 0; i < len_layout; ++i) {
    layout[i] = deck[start+i];
  }
  start += len_layout;
  for (size_t i = 0; i < len_stock; ++i) {
    stock[i] = deck[start+i];
  }
}

int GetUserNumberChoice(size_t len_options, size_t options[len_options]) {
  size_t count = 0;
  for (size_t i = 0; i < len_options; ++i) {
    if (options[i]) {
      count += 1;
    }
  }
  int choice = -1;
  size_t is_sequential = (count == len_options && len_options != 1);
  for (;;) {
    printf("選択[");
    if (is_sequential) {
      printf("%d-%lu", 0, len_options - 1);  
    } else {
      size_t current_count = 0;
      for (size_t i = 0; i < len_options; ++i) {
        if (options[i]) {
          printf("%lu", i);
          current_count += 1;
          if (current_count < count) {
            printf(",");
          }
        }
      }
    }
    printf("]：");
    char buffer[buffer_max] = { 0 };
    if (fgets(buffer, buffer_max, stdin) == 0) {
      continue;
    }
    int r = strtod(buffer, 0);
    int found = 0;
    for (size_t i = 0; i < len_options; ++i) {
      if (options[i] && r == i) {
        printf("選択済：%d\n", r);
        found = 1;
        break;
      }
    }
    if (found) {
      choice = r; 
      break;
    }
  }
  return choice;
}

int PickCardInHandByComputer(size_t len, hanafuda fuda[len]) {
  // 戦略：手札の中で最も得点の大きい札を選ぶ
  int max_point = -1;
  int largest_card_index = -1;
  for (size_t i = 0; i < len; ++i) {
    if (fuda[i].category > max_point) {
      max_point = fuda[i].category;
      largest_card_index = i;
    }
  }
  return largest_card_index;
}

int PickCardInHandByUser(size_t len, hanafuda fuda[len]) {
  size_t options[hand] = { 0 };
  for (size_t i = 0; i < len; ++i) {
    options[i] = 1;
  }
  int selected_index = GetUserNumberChoice(len, options);
  return selected_index;
}

int PickCardInHand(size_t len, hanafuda fuda[len], size_t is_computer) {
  if (is_computer) {
    return PickCardInHandByComputer(len, fuda);
  } else {
    return PickCardInHandByUser(len, fuda);
  }
}

void PrintNewLine() {
  printf("\n");
}

void PrintCardByEachLine(hanafuda fuda, size_t line, int is_face_up) {
  if (line == 0) {
    printf("┌──┐");
  } else if (line == 1 || line == 2) {
    if (is_face_up) {
      int mi = fuda.month_index;
      size_t li = fuda.local_index;
      fprintf(stdout, "│%s│", kCardChar[line-1][mi][li]);
    } else {
      printf("│∬∬│");
    }
  } else if (line == 3) {
    printf("└──┘");
  }
}

void PrintOneCard(hanafuda fuda) {
  int is_face_up = 1;
  for (size_t line = 0; line < 4; ++line) {
    PrintCardByEachLine(fuda, line, is_face_up);
    PrintNewLine();
  }
}

void RenderUpcard(size_t len, hanafuda fuda[len], int is_banker) {
  int is_face_up = 1;
  if (is_banker) {
    puts("親の持ち札");
  } else {
    puts("子の持ち札");
  }
  if (!len) {
    printf("\n\n\n\n");
  } else {
    for (size_t line = 0; line < 4; ++line) {
      for (size_t i = 0; i < len; ++i) {
        PrintCardByEachLine(fuda[i], line, is_face_up);
      }
      PrintNewLine();
    }
  }
}

void RenderLayout(size_t len, hanafuda fuda[len]) {
  puts("場札");
  int is_face_up = 1;
  for (size_t line = 0; line < 4; ++line) {
    for (size_t i = 0; i < len; ++i) {
      PrintCardByEachLine(fuda[i], line, is_face_up);
    }
    PrintNewLine();
  }
  for (size_t i = 0; i < len; ++i) {
    printf("%3lu ", i);
  }
  PrintNewLine();
}

void RenderHand(size_t len, hanafuda fuda[len],
                int is_face_up, int is_banker, int with_number) {
  if (is_banker) {
    puts("親の手札");
  } else {
    puts("子の手札");
  }
  for (size_t line = 0; line < 4; ++line) {
    for (size_t i = 0; i < len; ++i) {
      PrintCardByEachLine(fuda[i], line, is_face_up);
    }
    PrintNewLine();
  }
  if (with_number) {
    for (size_t i = 0; i < len; ++i) {
      printf("%3lu ", i);
    }
    PrintNewLine();
  }
}

void RenderStock(size_t len_stock, hanafuda stock[len_stock]) {
  printf("\n"
         " 山札\n"
         " ┌──┐\n"
         " │%2lu│┐\n"
         " │∬∬││\n"
         " └──┘│\n"
         "  └──┘\n\n", len_stock);
}

void RenderHorizontalLine(size_t line_len) {
  for (size_t i = 0; i < line_len; ++i) {
    printf("╌");
  }
  PrintNewLine();
}

void Render(size_t len_stock, hanafuda stock[len_stock],
            size_t len_punter_hand, hanafuda punter_hand[len_punter_hand],
            size_t len_punter_upcard, hanafuda punter_upcard[len_punter_upcard],
            size_t len_layout, hanafuda layout[len_layout],
            size_t len_banker_upcard, hanafuda banker_upcard[len_banker_upcard],
            size_t len_banker_hand, hanafuda banker_hand[len_banker_hand],
            size_t turn, size_t player_id) {
  int is_face_up, is_banker, with_number;
  size_t line_len = 32;
  RenderHorizontalLine(line_len);
  if (player_id == banker_player) {
    printf("%lu回目 親", turn);
  } else {
    printf("%lu回目 子", turn);
  }
  RenderStock(len_stock, stock);
  is_face_up = 0;
  is_banker = 0;
  with_number = 0;
  RenderHand(len_punter_hand, punter_hand, is_face_up, is_banker, with_number);
  RenderUpcard(len_punter_upcard, punter_upcard, is_banker);
  RenderLayout(len_layout, layout);
  is_face_up = 1;
  is_banker = 1;
  with_number = 1;
  RenderUpcard(len_banker_upcard, banker_upcard, is_banker);
  RenderHand(len_banker_hand, banker_hand, is_face_up, is_banker, with_number);
}

size_t MoveOverCards(size_t len, hanafuda fuda[len], size_t deleted_index) {
  for (size_t i = 0; i < len; ++i) {
    if (i <= deleted_index) {
      continue;
    } else {
      fuda[i-1] = fuda[i];
    }
  }
  return len - 1;
}

int PickCardInLayoutByComputer(size_t len, hanafuda fuda[len], size_t options[len]) {
  int max_point = -1;
  int largest_card_index = -1;
  for (size_t i = 0; i < len; ++i) {
    if (options[i] && (fuda[i].category > max_point)) {
      max_point = fuda[i].category;
      largest_card_index = i;
    }
  }
  return largest_card_index;
}

int PickCardInLayoutByUser(size_t len, size_t options[len]) {
  return GetUserNumberChoice(len, options);
}

int PickSameMonthCardInLayout(hanafuda needle, size_t len,
                              hanafuda haystack[len], int is_computer) {
  size_t options[full] = { 0 };
  size_t month_index = needle.month_index;
  // 場札のサイズはゲームの進行につれ増減する
  // malloc()/free()を避けるために最大幅 full を確保しているが、実際に使うのは len までで十分
  int match = 0;
  for (size_t i = 0; i < len; ++i) {
    if (haystack[i].month_index == month_index) {
      options[i] = 1;
      match += 1;
    }
  }
  if (!match) {
    return -1;
  } else {
    if (is_computer) {
      return PickCardInLayoutByComputer(len, haystack, options);
    } else {
      return PickCardInLayoutByUser(len, options);
    }
  }
}

void CountCards(size_t len_state, size_t state[len_state],
                size_t len_upcard, hanafuda upcard[len_upcard]) {
  for (size_t i = 0; i < len_upcard; ++i) {
    state[upcard[i].category] += 1;
  }
}

void Score(size_t len_state, size_t state[len_state],
           size_t len_meld, size_t meld[len_meld]) {
  if (state[hikari] == 4 && ame == 1) {
    meld[goko] = 1;
  } else if (state[hikari] == 4 && ame == 0) {
    meld[shiko] = 1;
  } else if (state[hikari] == 3 && ame == 1) {
    meld[ameshiko] = 1;
  } else if (state[hikari] == 3 && ame == 0) {
    meld[sanko] = 1;
  }
  if (state[maku] == 1 && state[sakazuki] == 1) {
    meld[hanami] = 1;
  }
  if (state[tsuki] == 1 && state[sakazuki] == 1) {
    meld[tsuki] = 1;
  }
  if (state[inoshishi] == 1 && state[shika] == 1 && state[cho] == 1) {
    meld[inoshikacho] = 1;
  }
  if (state[aka] >= 3) {
    meld[akatan] = 1;
  }
  if (state[ao] >= 3) {
    meld[aotan] = 1;
  }
  if (state[tanefuda] + state[cho] + state[inoshishi] + state[sakazuki] + state[shika] >= 5) {
    meld[tane] = 1;
  }
  if (state[tanzaku] + state[ao] + state[aka] >= 5) {
    meld[tan] = 1;
  }
  if (state[kasufuda] >= 10) {
    meld[kasu] = 1;
  }
}

int AskEndGameToComputer() {
  srand(time(0));
  int r = rand() * 2 % 2;
  if (!r) {
    puts("こいこい");
  }
  return r;
}

int AskEndGameToUser() {
  int koikoi = 0;
  int end_game = 0;
  for (;;) {
    printf("こいこいしますか？[yN]：");
    char buffer[buffer_max] = { 0 };
    if (fgets(buffer, buffer_max, stdin) == 0) {
      continue;
    }
    buffer[1] = '\0';
    if (!strcmp(buffer, "y")) {
      puts("yesを選択");
      koikoi = 1;
      break;
    } else if (!strcmp(buffer, "N")) {
      koikoi = 0;
      break;
    }
  }
  if (koikoi) {
    end_game = 0;
    puts("こいこい");
  } else {
    end_game = 1;
  }
  return end_game;
}

int AskEndGame(size_t is_computer) {
  if (is_computer) {
    return AskEndGameToComputer();
  } else {
    return AskEndGameToUser();
  }
}

int FindNewMeld(size_t len_previous_meld, size_t previous_meld[len_previous_meld],
                size_t len_meld, size_t meld[len_meld]) {
  int new_meld_count = 0;
  int diff = 0;
  for (size_t i = 0; i < len_previous_meld; ++i) {
    diff = meld[i] - previous_meld[i];
    if (diff) {
      fprintf(stdout, "%s\n", kMeldLabel[i]);
    }
    new_meld_count += diff;
  }
  return new_meld_count;
}

int FindNewMeldAndAskEndGame(size_t len_previous_meld, size_t previous_meld[len_previous_meld],
                               size_t len_meld, size_t meld[len_meld], size_t is_computer) {
  int end_game = 0;
  int new_meld_count = FindNewMeld(len_previous_meld, previous_meld, len_meld, meld);
  if (new_meld_count) {
    end_game = AskEndGame(is_computer); 
  }
  return end_game;
}

void StartGame() {
  hanafuda deck[full];
  hanafuda hands[player_size][hand];
  hanafuda layout[full];
  hanafuda stock[half];
  hanafuda upcards[player_size][hand];
  size_t melds[player_size][meld_size] = { { 0 } };
  size_t len_deck = full;
  size_t len_hands[player_size] = {hand, hand};
  size_t len_upcards[player_size] = {empty, empty};
  size_t len_layout = hand;
  size_t len_stock = half;

  InitDeck(len_deck, deck);
  size_t times = 1000;
  ShuffleDeck(len_deck, deck, times);
  Deal(len_deck, deck, len_hands[banker_player], hands[banker_player],
       len_hands[punter_player], hands[punter_player], len_layout, layout, len_stock, stock);
  for (size_t turn = 0; turn < hand; ++turn) {
    int end_game = 0;
    // 親と子が交互に
    for (size_t player_id = 0; player_id < player_size; ++player_id) {
      size_t is_computer = (player_id == punter_player);
      Render(len_stock, stock,
             len_hands[punter_player], hands[punter_player],
             len_upcards[punter_player], upcards[punter_player],
             len_layout, layout,
             len_upcards[banker_player], upcards[banker_player],
             len_hands[banker_player], hands[banker_player],
             turn, player_id);
      // 手札を出す
      int picked_index = PickCardInHand(len_hands[player_id], hands[player_id], is_computer);
      hanafuda picked = hands[player_id][picked_index];
      len_hands[player_id] = MoveOverCards(len_hands[player_id], hands[player_id], picked_index);
      // 同じ月の札があれば合札、なければ捨て札 
      int matched_index = PickSameMonthCardInLayout(picked, len_layout, layout, is_computer);
      if (matched_index >= 0) {
        upcards[player_id][len_upcards[player_id]] = picked;
        len_upcards[player_id] += 1;
        upcards[player_id][len_upcards[player_id]] = layout[matched_index];
        len_layout = MoveOverCards(len_layout, layout, matched_index);
        len_upcards[player_id] += 1;
      } else {
        layout[len_layout] = picked;
        len_layout += 1;
      }
      // 山札を引く
      int first = 0;
      picked = stock[first];
      len_stock = MoveOverCards(len_stock, stock, first);
      PrintOneCard(picked);
      // 同じ月の札があれば合札、なければ捨て札
      matched_index = PickSameMonthCardInLayout(picked, len_layout, layout, is_computer);
      if (matched_index >= 0) {
        upcards[player_id][len_upcards[player_id]] = picked;
        len_upcards[player_id] += 1;
        upcards[player_id][len_upcards[player_id]] = layout[matched_index];
        len_layout = MoveOverCards(len_layout, layout, matched_index);
        len_upcards[player_id] += 1;
      } else {
        layout[len_layout] = picked;
        len_layout += 1;
      }
      // 出来役があれば「こいこい」で継続か終局
      size_t state[category_size] = { 0 };
      CountCards(category_size, state, len_upcards[player_id], upcards[player_id]);
      size_t previous_meld[meld_size];
      for (size_t i = 0; i < meld_size; ++i) {
        previous_meld[i] = melds[player_id][i];
      }
      Score(category_size, state, meld_size, melds[player_id]);
      end_game = FindNewMeldAndAskEndGame(meld_size, previous_meld,
                                                       meld_size, melds[player_id], is_computer);
      if (end_game) {
        if (player_id == banker_player) {
          puts("親の勝ち");
        } else {
          puts("子の勝ち");
        }
        break;
      }
    }
    if (end_game) {
      break;
    }
  }
}

int main() {
  StartGame(); 
  return EXIT_SUCCESS;
}
