/*ЗАДАЧА
Дан набор файлов первой папки и второй
надо определить, какие файлы похожи на некоторые файлы второй (длина наибольшей общей подстроки)
найти идентичные файлы
найти уникальные файлы для каждой папки.
*/

#include <iostream>
#include <vector>
#include <fstream>

#include <dirent.h>

#include <algorithm>
#include <set>

#include <filesystem>

namespace fs {
    using namespace std::filesystem;
};

struct RefrenResult {
    int answer;
    std::string str;
    int l;
    int r;
    int len;
    RefrenResult() : answer(0) {}
};

class SuffixArray {
    std::vector<std::pair<int, int>> suff_array;
    std::vector<int> str;
    int str_len;
    std::vector<int> cls;

    void ZeroStepSort();
    void ExpandArray(int deg);

public:

    SuffixArray(const std::vector<int>& _text);
    void Print() const;
    const int& operator[](size_t idx) const {
        return suff_array[idx].second;
    }

    const std::vector<int>& get_str() const {
        return str;
    }
};

void SuffixArray::ZeroStepSort() {
    std::vector<std::pair<char, int>> indexed_text(str_len);

    for (int i = 0; i < str_len; ++i) {
        indexed_text[i] = std::make_pair(str[i], i);
    }

    std::sort(indexed_text.begin(), indexed_text.end());
    int cls_cnt = 0;
    char prev_letter = indexed_text[0].first;

    for (const auto& [letter, index] : indexed_text) {
        suff_array.emplace_back((letter > prev_letter? ++cls_cnt : cls_cnt), index);
        cls[index] = cls_cnt;
        prev_letter = letter;
    }
}

void SuffixArray::ExpandArray(int deg) {
    for (int i = 0; i < str_len; ++i) {
        int index = (str_len + (suff_array[i].second - (1 << deg))) % str_len;
        suff_array[i] = std::make_pair(cls[index], index);
    }
}

SuffixArray::SuffixArray(const std::vector<int>& str)
        : str(str), str_len(static_cast<int>(str.size()) + 1),
          cls(str.size() + 1, 0) {
    this->str.push_back(100000);
    suff_array.reserve(str_len);
    ZeroStepSort();
    std::vector<int> cls_cnt(str_len, 0);
    std::vector<std::pair<int, int>> _suff_array(str_len);

    for (int deg = 0; (1 << deg) < str_len; ++deg) {
        cls_cnt.assign(str_len, 0);
        ExpandArray(deg);

        for (auto suff : suff_array) {
            ++cls_cnt[suff.first];
        }

        for (int i = 1; i < str_len; ++i) {
            cls_cnt[i] += cls_cnt[i - 1];
        }

        for (int i = str_len - 1; i >= 0; --i) {
            _suff_array[--cls_cnt[suff_array[i].first]] = suff_array[i];
        }

        suff_array = _suff_array;
        cls_cnt = cls;
        int counter = 0;
        cls[0] = 0;
        suff_array[0].first = 0;

        for (size_t i = 1; i < str_len; ++i) {

            size_t l = (suff_array[i - 1].second + (1 << deg)) % str_len;
            size_t r = (suff_array[i].second + (1 << deg)) % str_len;

            if (cls_cnt[suff_array[i - 1].second] < cls_cnt[suff_array[i].second] ||
                cls_cnt[l] < cls_cnt[r]) {
                ++counter;
            }

            suff_array[i].first = counter;
            cls[suff_array[i].second] = counter;
        }
    }

}

void SuffixArray::Print() const {
    for (const auto& [letter, index] : suff_array) {
        std::cout << std::string(str.begin() + index, str.end());
        std::cout << std::endl;
    }
}

std::vector<int> BuildLCP(const SuffixArray& suf) {
    const std::vector<int>& str = suf.get_str();

    std::vector<int> lcp(str.size(), 0);
    size_t n = str.size();
    std::vector<int> opposite_suf(n);

    for (int i = 0; i < n; ++i) {
        opposite_suf[suf[i]] = i;
    }

    int cur_lcp = 0;
    int cur_suffix = 0;

    for (int i = 0; i < n; ++i) {
        if (cur_lcp) {
            --cur_lcp;
        }
        if (opposite_suf[i] == n - 1) {
            lcp[n - 1] = -1;
            cur_lcp = 0;
        }
        else {
            cur_suffix = suf[opposite_suf[i] + 1];

            while (std::max(i + cur_lcp, cur_suffix + cur_lcp) < n
                   && str[i + cur_lcp] == str[cur_suffix + cur_lcp]) {

                ++cur_lcp;
            }

            lcp[opposite_suf[i]] = cur_lcp;
        }
    }

    lcp.erase(lcp.end() - 1);

    return lcp;
}


int main() {
    std::string path1_str;
    std::string path2_str;

    std::cin >> path1_str;
    std::cin >> path2_str;

    int param = 0;
    std::cin >> param;

    std::set<std::string> paired1;
    std::set<std::string> paired2;

    int cnt1 = 0;
    int cnt2 = 0;

    for (auto file1: std::filesystem::directory_iterator(std::filesystem::path(path1_str))) {
        ++cnt1;
        for (auto file2: std::filesystem::directory_iterator(std::filesystem::path(path2_str))) {
            if (cnt1 == 1) {
                ++cnt2;
            }
            // Файл не более 2 гигабайт, пока так..

            FILE* fp1 = fopen(file1.path().c_str(), "r");
            if (fp1 == nullptr) {
                return 1;
            }
            std::vector<int> data1;
            int value = 0;
            while (true) {
                size_t num_read = fread(&value, sizeof(char), 1, fp1);
                if (feof(fp1) || ferror(fp1)) {
                    break;
                }
                data1.push_back(value);
            }
            fclose(fp1);


            FILE* fp2 = fopen(file2.path().c_str(), "r");
            if (fp2 == nullptr) {
                return 2;
            }
            std::vector<int> data2;
            value = 0;
            while (true) {
                size_t num_read = fread(&value, sizeof(char), 1, fp2);
                if (feof(fp2) || ferror(fp2)) {
                    break;
                }
                data2.push_back(value);
            }
            fclose(fp2);

            std::vector<int> concat = data1;
            concat.insert(concat.end(), data2.begin(), data2.end());

            SuffixArray sf(concat);
            std::vector<int> lcp = BuildLCP(sf);

            int max_len = 0;
            for (int pos = 0; pos < lcp.size(); ++pos) {
                // Если суффиксы из разных файлов в конкатенации, то общая длина -- lcp.
                if ((sf[pos] < data1.size()) != (sf[pos + 1] < data1.size())) {
                    max_len = std::max(max_len, lcp[pos]);
                }
            }

            auto file_len = std::max(std::filesystem::file_size(file1), std::filesystem::file_size(file2));

             if (file_len == max_len) {
                std::cout << path1_str << '/' << file2.path().filename().c_str() << " - " << path2_str << '/' << file2.path().filename().c_str() << '\n';
                paired1.insert(file1.path().filename().c_str());
                paired2.insert(file2.path().filename().c_str());
             } else {
                 int percent = (int)((float)max_len / file_len * 100);
                 if (percent >= param) {
                     std::cout << path1_str << '/' << file2.path().filename().c_str() << " - " << path2_str << '/' << file2.path().filename().c_str() << ' ' << (float)max_len / file_len * 100 << "%\n";
                     paired1.insert(file1.path().filename().c_str());
                     paired2.insert(file2.path().filename().c_str());
                 }
             }
        }
    }

    if (paired1.size() != (size_t)cnt1) {
        std::cout << "unique files first folder:\n";
    }

    for (auto file1: std::filesystem::directory_iterator(std::filesystem::path(path1_str))) {

        if (paired1.find(file1.path().filename().c_str()) == paired1.end()) {
            std::cout << path1_str << '/' << file1.path().filename().c_str() << '\n';
        }
    }

    if (paired2.size() != (size_t)cnt2) {
        std::cout << "unique files second folder:\n";
    }

    for (auto file2: std::filesystem::directory_iterator(std::filesystem::path(path2_str))) {
        if (paired2.find(file2.path().filename().c_str()) == paired2.end()) {
            std::cout << path2_str << '/' << file2.path().filename().c_str() << '\n';
        }
    }

    return 0;
}
