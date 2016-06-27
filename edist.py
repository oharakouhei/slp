# python3.5.1
import sys
import numpy as np

def MinimumEditDistance(object):
    def __init__(self):
        self.INS_COST = 1
        self.DEL_COST = 1
        self.SUBST_COST = 2

    def calc(self, source, target):
        '''
        target: 行方向
        source: 列方向
        '''
        # initialize
        distance = np.zeros((len(source)+1, len(target)+1))
        distance[0] = range(len(target)+1)
        distance[:,0] = range(len(source)+1)

        for tgt_id, tgt_char in enumerate(target):
            for src_id, src_char in enumerate(source):
                distance[src_id+1, tgt_id+1] = min(
                    distance[src_id, tgt_id+1]+self.INS_COST,
                    distance[src_id, tgt_id]+self.SUBST_COST,
                    distance[src_id+1, tgt_id]+self.DEL_COST,
                )

        return distance[len(source)+1, len(target)+1]

if __name__ == "__main__":
    print("Please input 2 words.")
    input_words = input("> ")
    arr = input_words.split()

    if len(arr) != 2:
        sys.stderr.write("It's not 2 words!\n")
        sys.exit(1)

    source_word = arr[0]
    target_word = arr[1]

    med = MinimumEditDistance()
    print(med.calc(source_word, target_word))
