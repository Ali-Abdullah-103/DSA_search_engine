#include "lexicon.hpp"
#include <iostream>

int main() {
    Lexicon lex;

    // Test adding words
    lex.add("apple");
    lex.add("banana", 3);
    lex.add("apple", 2);
    lex.add("cherry", 5);
    lex.add("date", 1);

    std::cout << "\n--- Lexicon Size ---\n";
    std::cout << "Total words: " << lex.size() << "\n";

    // Check presence
    std::cout << "\n--- Testing present_in() ---\n";
    std::cout << "Is 'apple' present? " << (lex.present_in("apple") ? "Yes" : "No") << "\n";
    std::cout << "Is 'orange' present? " << (lex.present_in("orange") ? "Yes" : "No") << "\n";

    // Get ID and Frequency
    std::cout << "\n--- Testing getID() and getFrequency() ---\n";
    std::cout << "ID of 'banana': " << lex.getID("banana") << "\n";
    std::cout << "Frequency of 'banana': " << lex.getFrequency("banana") << "\n";
    std::cout << "ID of 'orange': " << lex.getID("orange") << "\n"; // should show max size_t

    // Show top words
    std::cout << "\n--- Top 3 Words ---\n";
    lex.print_top_words(3);

    // Save to file
    std::cout << "\n--- Saving to file ---\n";
    lex.save("D:/lexicon_data.txt");
    std::cout << "Saved to lexicon_data.txt\n";

    // Clear and reload
    std::cout << "\n--- Clearing and Loading from file ---\n";
    lex.clear_lex();
    std::cout << "Size after clear: " << lex.size() << "\n";

    if (lex.load("D:/lexicon_data.txt")) {
        std::cout << "Reloaded successfully.\n";
    }

    std::cout << "Size after reload: " << lex.size() << "\n";

    // Print after loading
    std::cout << "\n--- Words after loading ---\n";
    lex.print_top_words(3);

    return 0;
}