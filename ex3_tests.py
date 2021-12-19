import os
import subprocess

EXECUTABLE = "./tweetsGenerator"


def check_tweet_starter(out_file):
    with open(out_file, 'r') as f:
        for i, line in enumerate(f.readlines()):
            if not line.startswith(f"Tweet {i + 1}: "):
                return False
    return True


def check_leaks():
    leaks = True
    with open("valgrind-out.txt", 'r') as valgrind_log:
        val_lines = valgrind_log.readlines()
        for line in val_lines:
            if "no leaks are possible" in line:
                leaks = False
        if leaks:
            print("valgrind found memory leaks, check your code for allocation/freeing errors and run the tests again")
            return True
    return False


def valgrind_test():
    print("running valgrind with full check and debug mode")
    with open("stdout.txt", 'w') as out_file:
        try:
            valgrind = subprocess.run(
                "valgrind --leak-check=full --tool=memcheck --show-leak-kinds=all --track-origins=yes --verbose "
                f"--error-exitcode=1 -v --log-file=valgrind-out.txt {EXECUTABLE} 12345 100 in_files/500_tweets.txt",
                stdout=out_file, text=True, shell=True, timeout=20)
        except subprocess.TimeoutExpired:
            print("Timeout occurred, test stopping")
            return False
    if check_leaks():
        return False

    if not check_tweet_starter("stdout.txt"):
        print("Test with args '12345 100 in_files/500_tweets.txt' failed.")
        print("This test checks for memory leaks and basic printing format.")
        print("Expected all lines to start with 'Tweet (num): ' see 'stdout.txt' for actual prints.")
        return False
    return True


def test_usage_error():
    with open("stdout.txt", 'w') as out_file:
        valgrind = subprocess.run(
            f"{EXECUTABLE} badargs",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file:
        res = out_file.readline()

    if not res.startswith("Usage: "):
        print("Test with args 'badargs' failed.")
        print("This test checks that you print a usage error when the argument count does not match.")
        print("Expected message to start with 'Usage: ' see 'stdout.txt' for actual.")
        return False
    return True


def test_enders():
    with open("stdout.txt", 'w') as out_file:
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 10 in_files/check_word_enders.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file, open("expected/check_word_enders.txt", 'r') as expected:
        res = out_file.readlines()
        e = expected.readlines()

    if res != e:
        print("Test with args '12345 10 in_files/check_word_enders.txt' failed.")
        print("This test checks that you do not start tweets with sentence enders.")
        print("See 'expected/check_word_enders.txt' for expected. See 'stdout.txt' for actual.")
        return False
    return True


def test_file_error():
    with open("stdout.txt", 'w') as out_file:
        proc = subprocess.run(
            f"{EXECUTABLE} 12345 10 unicorn.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    if proc.returncode == 139:
        print("You had a segfault when handling a non existing file.")

    with open("stdout.txt", 'r') as out_file:
        res = out_file.readline()

    if not res.startswith("Error: "):
        print("Test with args '12345 1 unicorn.txt' failed.")
        print("This test checks that you print an error when the text file can't be accessed.")
        print("Expected message to start with 'Error: ' see 'stdout.txt' for actual.")
        return False
    return True


def test_word_count():
    with open("stdout.txt", 'w') as out_file:
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 10 in_files/check_word_count.txt 2",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file, open("expected/check_word_count.txt", 'r') as expected:
        res = out_file.readlines()
        e = expected.readlines()

    if res != e:
        print("Test with args '12345 10 in_files/check_word_count.txt 2' failed.")
        print("This test checks that you read only the required amount of words from the corpus.")
        print("See 'expected/check_word_count.txt' for expected. See 'stdout.txt' for actual.")
        return False
    return True


def test_tweet_count():
    with open("stdout.txt", 'w') as out_file:
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 10 in_files/check_word_count.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file:
        res = out_file.readlines()

    if len(res) != 10:
        print("Test with args '12345 10 in_files/check_word_count.txt' failed.")
        print("This test checks that you generate the required amount of tweets.")
        print(f"Expected 10 tweets, and you generated {len(res)}. See 'stdout.txt' for details.")
        return False
    return True


def test_read_everything():
    with open("stdout.txt", 'w') as out_file:
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 100 in_files/check_word_count.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file:
        res = out_file.readlines()

    found_first = False
    found_second = False

    for r in res:
        if "but not this." in r:
            found_second = True
        elif "read this." in r:
            found_first = True
        if found_second and found_first:
            break

    if not (found_first and found_second):
        print("Test with args '12345 100 in_files/check_word_count.txt' failed.")
        print("This test checks that when no number is specified, you read the entire corpus.")
        print("Expected both tweets from 'expected/check_word_count.txt' to be present. See 'stdout.txt' for actual.")
        return False
    return True


def test_prob():
    with open("stdout.txt", 'w') as out_file:
        # Change 12345 to something else here if you need to
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 1000 in_files/prob_test.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file:
        res = out_file.readlines()

    small_counter = 0
    for i, r in enumerate(res):
        if r.strip().rstrip() == f"Tweet {i + 1}: prob small.":
            small_counter += 1

    if not (250 <= small_counter <= 350):
        print("Test with args '12345 1000 in_files/prob_test.txt' failed.")
        print("This test checks that you properly calculate probabilities for word appearances.")
        print(f"Expected between 250 and 350 appearances of 'prob small.'; actual was {small_counter}")
        return False
    return True


def test_prob_first_word():
    with open("stdout.txt", 'w') as out_file:
        # Change 12345 to something else here if you need to
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 1000 in_files/first_word_prob_test.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file:
        res = out_file.readlines()

    counter = {
        "first": 0,
        "second": 0,
        "third": 0,
        "fourth": 0,
        "fifth": 0,
        "sixth": 0,
        "seventh": 0,
        "eighth": 0,
        "ninth": 0,
        "tenth": 0
    }
    for r in res:
        for word in counter:
            if word in r:
                counter[word] += 1

    if not (all(70 <= c <= 130 for c in counter.values())):
        print("Test with args '12345 1000 in_files/first_word_prob_test.txt' failed.")
        print("This test checks that the probability to start with each word is equal.")
        print(f"Expected between 70 and 130 appearances of each word; actual was:")
        print(counter)
        return False
    return True


def test_seed():
    with open("stdout.txt", 'w') as out_file:
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 100 in_files/500_tweets.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout.txt", 'r') as out_file:
        res1 = out_file.readlines()

    with open("stdout2.txt", 'w') as out_file:
        valgrind = subprocess.run(
            f"{EXECUTABLE} 12345 100 in_files/500_tweets.txt",
            stdout=out_file, text=True, shell=True, timeout=5)

    with open("stdout2.txt", 'r') as out_file:
        res2 = out_file.readlines()

    if res2 != res1:
        print("Test with args '12345 100 in_files/500_tweets.txt' failed.")
        print("This test checks that you take the seed from the command line arguments, by running twice with the "
              "same arguments and comparing the results.")
        print("Expected both files to be equal, see 'stdout.txt' and 'stdout2.txt' for details.")
        return False
    return True


def setup():
    if os.path.isfile(EXECUTABLE):
        os.remove(EXECUTABLE)

    c = subprocess.run(
        f"gcc -w -Wextra -Wall -Wvla -std=c99 -lm {EXECUTABLE}.c -o {EXECUTABLE}",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        shell=True)

    if c.stdout != b'':
        print("\nSomething didn't go right when compiling your C source "
              "code! Output was: \n" + str(c.stdout))
        exit()


def main():
    print("Compiling...")
    setup()
    print("Done.")
    tests_passed = 0
    tests = [valgrind_test, test_usage_error, test_file_error, test_enders,
             test_prob,  # If you fail on the probability test, comment out this line.
             test_word_count, test_tweet_count, test_read_everything, test_seed, test_prob_first_word]

    for t in tests:
        if t():
            tests_passed += 1
        else:
            break
    print(f"Passed {tests_passed}/{len(tests)} tests.")


if __name__ == '__main__':
    main()
