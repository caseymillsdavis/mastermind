#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <set>
#include <array>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

#define NUM_PLACES 4
#define NUM_VALUES 8
#define SET_SIZE 4096 // 8^4

//#define NUM_PLACES 4
//#define NUM_VALUES 4
//#define SET_SIZE 256 // 3^4

//#define NUM_PLACES 4
//#define NUM_VALUES 3
//#define SET_SIZE 81 // 3^4

//#define NUM_PLACES 4
//#define NUM_VALUES 2
//#define SET_SIZE 16 // 2^4

typedef std::array<unsigned, NUM_PLACES> entry_t;
typedef std::vector<unsigned> h_vector_t;

typedef enum
{
    kBlank,
    kBlow,
    kHit
} info_e;


unsigned element_to_index(const entry_t &e)
{
    unsigned val = 0;
    unsigned m = 1;
    for (unsigned j = 0; j < NUM_PLACES; j++)
    {
        val += m*e[j];
        m *= NUM_VALUES;
    }
    return val;
}

void index_to_element(unsigned idx, entry_t *e)
{
    unsigned val = idx;
    for (unsigned j = 0; j < NUM_PLACES; j++)
    {
        (*e)[j] = val % NUM_VALUES;
        val /= NUM_VALUES;
    }
}

void index_to_element_str(unsigned idx, std::string *s)
{
    unsigned val = idx;
    for (unsigned j = 0; j < NUM_PLACES; j++)
    {
        s->insert(0,1,'0' + val % NUM_VALUES);
        val /= NUM_VALUES;
    }
}

void construct_initial_possible_set(h_vector_t *s)
{
    for (unsigned i = 0; i < SET_SIZE; i++)
    {
        s->push_back(i);
    }
}

void print_entry(unsigned e)
{
    entry_t entry;
    index_to_element(e, &entry);
    for (unsigned i = 0; i < NUM_PLACES; i++)
    {
        printf("%u ", entry[NUM_PLACES - i - 1]);
    }
}

void print_set(h_vector_t *s)
{
    h_vector_t::iterator it;
    unsigned index = 0;
    for (it = s->begin(); it != s->end(); it++)
    {
        entry_t entry;
        printf("%4u: ", index);
        index++;
        index_to_element(*it, &entry);
        for (unsigned i = 0; i < NUM_PLACES; i++)
        {
            printf("%u ", entry[i]);
        }
        printf("\n");
    }
}

void get_result(const entry_t &in, const entry_t &sol, std::string *res)
{
    info_e info[NUM_PLACES];
    unsigned hits = 0;
    unsigned blows = 0;

    /* Initialize blanks. */
    for (unsigned i = 0; i < NUM_PLACES; i++)
    {
        info[i] = kBlank;
    }

    /* Check for hits. */
    for (unsigned i = 0; i < NUM_PLACES; i++)
    {
        if (in[i] == sol[i])
        {
            info[i] = kHit;
            hits++;
            res->push_back('B');
        }
    }

    /* Check for blows. */
    for (unsigned i = 0; i < NUM_PLACES; i++)
    {
        if (info[i] == kHit) continue;
        for (unsigned j = 0; j < NUM_PLACES; j++)
        {
            if (info[j] != kBlank)
            {
                continue;
            }

            if (in[i] == sol[j])
            {
                info[j] = kBlow;
                blows++;
                res->push_back('W');
                break;
            }
        }
    }

    assert(hits + blows <= NUM_PLACES);
}

typedef std::unordered_map<std::string, h_vector_t> compatible_table_t;
typedef std::unordered_map<std::string, unsigned> hist_t;

void hist_insert(const std::string &s, hist_t *h)
{
    auto search = h->find(s);
    if (search != h->end())
    {
        (*h)[s] = (*h)[s] + 1;
    }
    else
    {
        (*h)[s] = 1;
    }
}

void compatible_table_insert(const std::string &s, compatible_table_t *t, unsigned e)
{
    auto search = t->find(s);
    if (search == t->end())
    {
        (*t)[s] = h_vector_t();
    }
    ((*t)[s]).push_back(e);
}

double hist_entropy(const hist_t &h, unsigned count)
{
    double entropy = 0.f;
    for (auto const &hi : h)
    {
        double prob = (double)hi.second / (double)count;
        entropy -= prob*log2f(prob);
    }
    return entropy;
}

double hist_entropy_min(const hist_t &h, unsigned count)
{
    double min_entropy;
    double prob = (double)(h.begin())->second / (double)count;
    min_entropy = -1*log2f(prob);
    for (auto const &hi : h)
    {
        double entropy;
        double prob = (double)hi.second / (double)count;
        entropy = -1*log2f(prob);
        if (entropy < min_entropy)
        {
            min_entropy = entropy;
        }
    }
    return min_entropy;
}

typedef std::vector<std::string> result_table_t[SET_SIZE];

void build_result_table(const h_vector_t &in, result_table_t *t)
{
    for (auto it0 = in.begin(); it0 != in.end(); it0++)
    {
        entry_t e0;
        index_to_element(*it0, &e0);
        for (auto it1 = in.begin(); it1 != in.end(); it1++)
        {
            std::string hb_str;
            entry_t e1;
            index_to_element(*it1, &e1);
            get_result(e0, e1, &hb_str);
            (*t)[*it0].push_back(hb_str);
            //printf("%4s ", hb_str.c_str());
        }
        //printf("\n");
    }
}

void get_result_from_table(unsigned e0, unsigned e1, std::string *res, const result_table_t &t)
{
    *res = t[e0][e1];
}

void get_maxent_table(const h_vector_t &in0, const h_vector_t &in1, compatible_table_t *ct_out, unsigned *maxent_guess, double *maxent, const result_table_t &t)
{
    compatible_table_t ct;
    bool is_valid = false;

    for (auto it0 = in0.begin(); it0 != in0.end(); it0++)
    {
        hist_t h;
        double entropy;
        for (auto it1 = in1.begin(); it1 != in1.end(); it1++)
        {
            std::string hb_str;
            get_result_from_table(*it0, *it1, &hb_str, t);
            hist_insert(hb_str, &h);
        }
        //entropy = hist_entropy(h, in1.size());
        entropy = hist_entropy_min(h, in1.size());

        /* Add check for whether selection is valid. Replace equal entropies
         * with valid if possible. */
        if (it0 == in0.begin() || entropy > *maxent ) //|| (is_valid == false && entropy == *maxent))
        {
            *maxent = entropy;
            *maxent_guess = *it0;

            //if (std::find(in1.begin(), in1.end(), *it0) != in1.end())
            //{
            //    is_valid = true;
            //}
            //std::string es;
            //index_to_element_str(*it0, &es);
            //printf("%4s: %3u %f\n", es.c_str(), h.size(), entropy);
        }
    }

    for (auto it1 = in1.begin(); it1 != in1.end(); it1++)
    {
        std::string hb_str;
        get_result_from_table(*maxent_guess, *it1, &hb_str, t);
        compatible_table_insert(hb_str, &ct, *it1);
    }
    *ct_out = compatible_table_t(ct);

}

int main(void)
{
    h_vector_t possibles;
    result_table_t result_table;
    std::string success;
    unsigned max_guesses = 1;

    construct_initial_possible_set(&possibles);

    build_result_table(possibles, &result_table);

    for (unsigned i = 0; i < NUM_PLACES; i++)
    {
        success.push_back('B');
    }

    for (auto it = possibles.begin(); it != possibles.end(); it++)
    {
        unsigned guess_count = 1;
        h_vector_t initial_set = possibles;
        h_vector_t next_set = possibles;
        printf("solve: ");
        print_entry(*it);
        printf("\n");
        while (next_set.size() > 1)
        {
            unsigned maxent_guess;
            compatible_table_t ct;
            double maxent;
            double guessent;
            unsigned is_size = next_set.size();
            std::string hb_str;
            get_maxent_table(initial_set, next_set, &ct, &maxent_guess, &maxent, result_table);
            get_result_from_table(*it, maxent_guess, &hb_str, result_table);
            next_set = ct[hb_str];

            guessent = log2f(1.f / ((double)next_set.size() / (double)is_size));
            printf("    %u: ", guess_count);
            print_entry(maxent_guess);
            printf(" %5s %4f, %4f\n", hb_str.c_str(), maxent, guessent);

            if (hb_str != success) guess_count++;
        }
        printf("    solution: ");
        print_entry(*(next_set.begin()));
        printf(" (%u guesses)\n", guess_count);

        if (guess_count > max_guesses)
        {
            max_guesses = guess_count;
        }
    }
    printf("max guesses required: %u\n", max_guesses);
    return 0;
}
