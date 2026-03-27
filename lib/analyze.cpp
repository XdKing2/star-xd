#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>

// ── Helpers ───────────────────────────────────────────────────
std::string toLower(const std::string& s) {
    std::string r = s;
    for (char& c : r) c = std::tolower(c);
    return r;
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (char c : text) {
        if (std::isalpha(c) || c == '\'') {
            word += std::tolower(c);
        } else {
            if (word.size() > 2) words.push_back(word);
            word.clear();
        }
    }
    if (word.size() > 2) words.push_back(word);
    return words;
}

static const std::set<std::string> STOPWORDS = {
    "the","and","for","are","but","not","you","all","can","her","was","one",
    "our","out","day","get","has","him","his","how","its","may","new","now",
    "old","see","two","who","boy","did","she","use","way","had","let","man",
    "too","any","big","end","put","say","set","try","been","come","does",
    "each","from","have","here","just","like","long","make","many","more",
    "most","much","name","need","only","open","over","part","same","seem",
    "some","such","take","than","that","them","then","they","this","time",
    "very","when","will","with","your","also","back","call","came","down",
    "give","good","hand","help","high","into","keep","kind","know","last",
    "left","life","live","look","made","mean","move","must","next","once",
    "own","play","read","real","said","show","side","tell","think","turn",
    "used","want","well","went","were","what","while","why","work","world",
    "would","year","years","could","their","there","these","those","about",
    "after","again","along","another","because","before","between","both",
    "during","every","found","going","great","group","having","large","never",
    "often","other","place","point","right","since","small","still","under",
    "until","where","which","while","without"
};

// ── Flesch Reading Ease ───────────────────────────────────────
int countSyllables(const std::string& word) {
    int count = 0;
    bool lastVowel = false;
    static const std::string vowels = "aeiouAEIOU";
    for (char c : word) {
        bool isVowel = vowels.find(c) != std::string::npos;
        if (isVowel && !lastVowel) count++;
        lastVowel = isVowel;
    }
    if (word.size() > 2 && word.back() == 'e') count--;
    return std::max(1, count);
}

// ── Sentiment (simple keyword-based) ─────────────────────────
static const std::set<std::string> POSITIVE_WORDS = {
    "good","great","excellent","amazing","wonderful","fantastic","outstanding",
    "superb","brilliant","perfect","love","happy","joy","beautiful","best",
    "awesome","incredible","magnificent","positive","success","win","victory",
    "achieve","benefit","improve","trust","hope","peace","safe","strong",
    "smart","kind","helpful","useful","valuable","recommend","enjoy","pleased",
    "satisfied","delighted","excited","inspired","motivated","confident",
    "proud","grateful","thankful","blessed","lucky","fortunate","wonderful",
    "nice","pleasant","friendly","fun","interesting","creative","innovative",
    "effective","efficient","productive","powerful","impressive","remarkable"
};

static const std::set<std::string> NEGATIVE_WORDS = {
    "bad","terrible","awful","horrible","disgusting","worst","hate","ugly",
    "fail","failure","poor","wrong","broken","damage","dangerous","death",
    "die","evil","fear","hurt","kill","lie","loss","pain","problem","risk",
    "sad","sick","weak","worried","anger","angry","conflict","corrupt",
    "crime","crisis","dark","disaster","disease","enemy","error","false",
    "fraud","guilty","harm","illegal","inferior","injustice","negative",
    "never","no","none","nothing","offensive","poor","refuse","reject",
    "shame","stupid","ugly","unhappy","useless","violence","war","waste",
    "wrong","destroy","attack","abuse","blame","broken","complaint","curse",
    "difficult","dirty","doubt","dread","dull","fake","nasty","nightmare"
};

struct AnalysisResult {
    int totalWords;
    int uniqueWords;
    int totalChars;
    int totalCharsNoSpaces;
    int sentences;
    int paragraphs;
    int syllables;
    double avgWordLen;
    double avgSentenceLen;
    double fleschScore;
    std::string readingLevel;
    std::string readingTime;
    int positiveWords;
    int negativeWords;
    std::string sentiment;
    double sentimentScore;
    std::vector<std::pair<std::string,int>> topWords;
    int longWords; // words > 6 chars
};

AnalysisResult analyze(const std::string& text) {
    AnalysisResult r = {};

    // Count sentences
    for (char c : text) {
        if (c == '.' || c == '!' || c == '?') r.sentences++;
    }
    if (r.sentences == 0) r.sentences = 1;

    // Count paragraphs
    std::istringstream paraStream(text);
    std::string line;
    std::string lastLine;
    while (std::getline(paraStream, line)) {
        if (trim(line).empty() && !trim(lastLine).empty()) r.paragraphs++;
        lastLine = line;
    }
    if (!trim(lastLine).empty()) r.paragraphs++;
    if (r.paragraphs == 0) r.paragraphs = 1;

    // Tokenize
    auto words = tokenize(text);
    r.totalWords = words.size();

    // Chars
    r.totalChars = text.size();
    for (char c : text) if (c != ' ' && c != '\t' && c != '\n') r.totalCharsNoSpaces++;

    // Unique words & freq map
    std::map<std::string, int> freq;
    std::set<std::string> seen;
    for (const auto& w : words) {
        freq[w]++;
        seen.insert(w);
    }
    r.uniqueWords = seen.size();

    // Avg word len
    int totalLen = 0;
    for (const auto& w : words) totalLen += w.size();
    r.avgWordLen = r.totalWords > 0 ? (double)totalLen / r.totalWords : 0;

    // Avg sentence len
    r.avgSentenceLen = r.totalWords > 0 ? (double)r.totalWords / r.sentences : 0;

    // Syllables
    for (const auto& w : words) r.syllables += countSyllables(w);

    // Flesch Reading Ease
    if (r.totalWords > 0) {
        double asl = r.avgSentenceLen;
        double asw = (double)r.syllables / r.totalWords;
        r.fleschScore = 206.835 - (1.015 * asl) - (84.6 * asw);
        r.fleschScore = std::max(0.0, std::min(100.0, r.fleschScore));
    }

    // Reading level
    if (r.fleschScore >= 90)      r.readingLevel = "Very Easy (5th grade)";
    else if (r.fleschScore >= 80) r.readingLevel = "Easy (6th grade)";
    else if (r.fleschScore >= 70) r.readingLevel = "Fairly Easy (7th grade)";
    else if (r.fleschScore >= 60) r.readingLevel = "Standard (8th-9th grade)";
    else if (r.fleschScore >= 50) r.readingLevel = "Fairly Difficult (10th-12th grade)";
    else if (r.fleschScore >= 30) r.readingLevel = "Difficult (College level)";
    else                          r.readingLevel = "Very Difficult (Professional)";

    // Reading time (200 wpm)
    int secs = (int)((r.totalWords / 200.0) * 60);
    if (secs < 60) r.readingTime = std::to_string(secs) + " seconds";
    else r.readingTime = std::to_string(secs/60) + "m " + std::to_string(secs%60) + "s";

    // Sentiment
    for (const auto& w : words) {
        if (POSITIVE_WORDS.count(w)) r.positiveWords++;
        if (NEGATIVE_WORDS.count(w)) r.negativeWords++;
    }
    int sentDiff = r.positiveWords - r.negativeWords;
    int sentTotal = r.positiveWords + r.negativeWords;
    r.sentimentScore = sentTotal > 0 ? (double)sentDiff / sentTotal * 100 : 0;
    if (r.sentimentScore > 20)       r.sentiment = "Positive 😊";
    else if (r.sentimentScore > 5)   r.sentiment = "Slightly Positive 🙂";
    else if (r.sentimentScore > -5)  r.sentiment = "Neutral 😐";
    else if (r.sentimentScore > -20) r.sentiment = "Slightly Negative 😕";
    else                             r.sentiment = "Negative 😞";

    // Long words
    for (const auto& w : words) if (w.size() > 6) r.longWords++;

    // Top 20 words (exclude stopwords)
    std::vector<std::pair<std::string,int>> freqVec;
    for (const auto& p : freq) {
        if (!STOPWORDS.count(p.first)) freqVec.push_back(p);
    }
    std::sort(freqVec.begin(), freqVec.end(),
        [](const auto& a, const auto& b){ return b.second < a.second; });
    for (int i = 0; i < std::min(20, (int)freqVec.size()); i++) {
        r.topWords.push_back(freqVec[i]);
    }

    return r;
}

void printJSON(const AnalysisResult& r) {
    std::cout << "{\n";
    std::cout << "  \"total_words\": " << r.totalWords << ",\n";
    std::cout << "  \"unique_words\": " << r.uniqueWords << ",\n";
    std::cout << "  \"total_chars\": " << r.totalChars << ",\n";
    std::cout << "  \"chars_no_spaces\": " << r.totalCharsNoSpaces << ",\n";
    std::cout << "  \"sentences\": " << r.sentences << ",\n";
    std::cout << "  \"paragraphs\": " << r.paragraphs << ",\n";
    std::cout << "  \"syllables\": " << r.syllables << ",\n";
    std::cout << "  \"avg_word_length\": " << std::fixed << std::setprecision(1) << r.avgWordLen << ",\n";
    std::cout << "  \"avg_sentence_length\": " << std::fixed << std::setprecision(1) << r.avgSentenceLen << ",\n";
    std::cout << "  \"flesch_score\": " << std::fixed << std::setprecision(1) << r.fleschScore << ",\n";
    std::cout << "  \"reading_level\": \"" << r.readingLevel << "\",\n";
    std::cout << "  \"reading_time\": \"" << r.readingTime << "\",\n";
    std::cout << "  \"sentiment\": \"" << r.sentiment << "\",\n";
    std::cout << "  \"sentiment_score\": " << std::fixed << std::setprecision(1) << r.sentimentScore << ",\n";
    std::cout << "  \"positive_words\": " << r.positiveWords << ",\n";
    std::cout << "  \"negative_words\": " << r.negativeWords << ",\n";
    std::cout << "  \"long_words\": " << r.longWords << ",\n";
    std::cout << "  \"top_words\": [\n";
    for (int i = 0; i < (int)r.topWords.size(); i++) {
        std::cout << "    {\"word\": \"" << r.topWords[i].first
                  << "\", \"count\": " << r.topWords[i].second << "}";
        if (i < (int)r.topWords.size() - 1) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "  ]\n";
    std::cout << "}\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: analyze <text...> | analyze --file <path>" << std::endl;
        return 1;
    }

    std::string text;
    if (std::string(argv[1]) == "--file") {
        if (argc < 3) { std::cerr << "No file path" << std::endl; return 1; }
        std::ifstream f(argv[2]);
        if (!f) { std::cerr << "Cannot open: " << argv[2] << std::endl; return 1; }
        text = std::string(std::istreambuf_iterator<char>(f), {});
    } else {
        for (int i = 1; i < argc; i++) {
            if (i > 1) text += " ";
            text += argv[i];
        }
    }

    if (text.empty()) {
        std::cerr << "Empty input" << std::endl;
        return 1;
    }

    auto result = analyze(text);
    printJSON(result);
    return 0;
}
