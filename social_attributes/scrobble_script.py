import urllib2
import json


rom __future__ import unicode_literals
import unittest
import re
import sys
import pep8

from fuzzywuzzy import fuzz
from fuzzywuzzy import process
from fuzzywuzzy import utils
from fuzzywuzzy.string_processing import StringProcessor
#songnames = [line.rstrip('\n') for line in open('songnames.txt')]
#artistnames = [line.rstrip('\n') for line in open('artistnames.txt')]




 def test_replace_non_letters_non_numbers_with_whitespace(self):
        strings = ["Believe If I",
                   "New York //// Mets $$$", "Ça va?"]
        for string in strings:
            proc_string = StringProcessor.replace_non_letters_non_numbers_with_whitespace(string)
            regex = re.compile(r"(?ui)[\W]")
            for expr in regex.finditer(proc_string):
                self.assertEqual(expr.group(), " ")

def partial_ratio(s1, s2):
    """"Return the ratio of the most similar substring
    as a number between 0 and 100."""
    s1, s2 = utils.make_type_consistent(s1, s2)

    if len(s1) <= len(s2):
        shorter = s1
        longer = s2
    else:
        shorter = s2
        longer = s1

    m = SequenceMatcher(None, shorter, longer)
    blocks = m.get_matching_blocks()

    # each block represents a sequence of matching characters in a string
    # of the form (idx_1, idx_2, len)
    # the best partial match will block align with at least one of those blocks
    #   e.g. shorter = "abcd", longer = XXXbcdeEEE
    #   block = (1,3,3)
    #   best score === ratio("abcd", "Xbcd")
    scores = []
    for block in blocks:
        long_start = block[1] - block[0] if (block[1] - block[0]) > 0 else 0
        long_end = long_start + len(shorter)
        long_substr = longer[long_start:long_end]

        m2 = SequenceMatcher(None, shorter, long_substr)
        r = m2.ratio()
        if r > .995:
            return 100
        else:
            scores.append(r)

    return utils.intr(100 * max(scores))

##############################
# Advanced Scoring Functions #
##############################
def _process_and_sort(s, force_ascii):
    """Return a cleaned string with token sorted."""
    # pull tokens
    tokens = utils.full_process(s, force_ascii=force_ascii).split()

    # sort tokens and join
    sorted_string = u" ".join(sorted(tokens))
    return sorted_string.strip()
# Sorted Token
#   find all alphanumeric tokens in the string
#   sort those tokens and take ratio of resulting joined strings
#   controls for unordered string elements

def _token_sort(s1, s2, partial=True, force_ascii=True):
    sorted1 = _process_and_sort(s1, force_ascii)
    sorted2 = _process_and_sort(s2, force_ascii)

    if partial:
        return partial_ratio(sorted1, sorted2)
    else:
        return ratio(sorted1, sorted2)


def token_sort_ratio(s1, s2, force_ascii=True):
    """Return a measure of the sequences' similarity between 0 and 100
    but sorting the token before comparing.
    """
    return _token_sort(s1, s2, partial=False, force_ascii=force_ascii)


def partial_token_sort_ratio(s1, s2, force_ascii=True):
    """Return the ratio of the most similar substring as a number between
    0 and 100 but sorting the token before comparing.
    """
    return _token_sort(s1, s2, partial=True, force_ascii=force_ascii)


def _token_set(s1, s2, partial=True, force_ascii=True):
    """Find all alphanumeric tokens in each string...
        - treat them as a set
        - construct two strings of the form:
            <sorted_intersection><sorted_remainder>
        - take ratios of those two strings
        - controls for unordered partial matches"""

    p1 = utils.full_process(s1, force_ascii=force_ascii)
    p2 = utils.full_process(s2, force_ascii=force_ascii)

    if not utils.validate_string(p1):
        return 0
    if not utils.validate_string(p2):
        return 0

    # pull tokens
    tokens1 = set(utils.full_process(p1).split())
    tokens2 = set(utils.full_process(p2).split())

    intersection = tokens1.intersection(tokens2)
    diff1to2 = tokens1.difference(tokens2)
    diff2to1 = tokens2.difference(tokens1)

    sorted_sect = " ".join(sorted(intersection))
    sorted_1to2 = " ".join(sorted(diff1to2))
    sorted_2to1 = " ".join(sorted(diff2to1))

    combined_1to2 = sorted_sect + " " + sorted_1to2
    combined_2to1 = sorted_sect + " " + sorted_2to1

    # strip
    sorted_sect = sorted_sect.strip()
    combined_1to2 = combined_1to2.strip()
    combined_2to1 = combined_2to1.strip()

    if partial:
        ratio_func = partial_ratio
    else:
        ratio_func = ratio

    pairwise = [
        ratio_func(sorted_sect, combined_1to2),
        ratio_func(sorted_sect, combined_2to1),
        ratio_func(combined_1to2, combined_2to1)
    ]
    return max(pairwise)


def token_set_ratio(s1, s2, force_ascii=True):
    return _token_set(s1, s2, partial=False, force_ascii=force_ascii)


def partial_token_set_ratio(s1, s2, force_ascii=True):
    return _token_set(s1, s2, partial=True, force_ascii=force_ascii)


###################
# Combination API #
###################

def QRatio(s1, s2, force_ascii=True):

    p1 = utils.full_process(s1, force_ascii=force_ascii)
    p2 = utils.full_process(s2, force_ascii=force_ascii)

    if not utils.validate_string(p1):
        return 0
    if not utils.validate_string(p2):
        return 0

    return ratio(p1, p2)


def UQRatio(s1, s2):
    return QRatio(s1, s2, force_ascii=False)


# w is for weighted
def WRatio(s1, s2, force_ascii=True):
    """Return a measure of the sequences' similarity between 0 and 100,
    using different algorithms.
    """

    p1 = utils.full_process(s1, force_ascii=force_ascii)
    p2 = utils.full_process(s2, force_ascii=force_ascii)

    if not utils.validate_string(p1):
        return 0
    if not utils.validate_string(p2):
        return 0

    # should we look at partials?
    try_partial = True
    unbase_scale = .95
    partial_scale = .90

    base = ratio(p1, p2)
    len_ratio = float(max(len(p1), len(p2))) / min(len(p1), len(p2))

    # if strings are similar length, don't use partials
    if len_ratio < 1.5:
        try_partial = False

    # if one string is much much shorter than the other
    if len_ratio > 8:
        partial_scale = .6

    if try_partial:
        partial = partial_ratio(p1, p2) * partial_scale
        ptsor = partial_token_sort_ratio(p1, p2, force_ascii=force_ascii) \
            * unbase_scale * partial_scale
        ptser = partial_token_set_ratio(p1, p2, force_ascii=force_ascii) \
            * unbase_scale * partial_scale

        return utils.intr(max(base, partial, ptsor, ptser))
    else:
        tsor = token_sort_ratio(p1, p2, force_ascii=force_ascii) * unbase_scale
        tser = token_set_ratio(p1, p2, force_ascii=force_ascii) * unbase_scale

        return utils.intr(max(base, tsor, tser))


def UWRatio(s1, s2):
    """Return a measure of the sequences' similarity between 0 and 100,
    using different algorithms. Same as WRatio but preserving unicode.
    """
    return WRatio(s1, s2, force_ascii=False)





outfile = open("./output.txt", "w")

url = "http://ws.audioscrobbler.com/2.0/?method=artist.gettoptracks&api_key=a02a7ba4c553ef83ef1ad7dc1911665b&format=json"
artistnames = ['cher']

for artist in artist_arr:
	temp_url = url + "&artist=" + artist
	response1 = urllib2.urlopen(temp_url).read()
	response = json.loads(response1)
	tracks = response["toptracks"]["track"]
	for i in tracks:
		print  >> outfile, str(i["name"].encode('ascii','ignore'))
outfile.close()


