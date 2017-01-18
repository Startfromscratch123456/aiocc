# description:   An objective function describes the goal of the optimization.
#                It generates a score that reflects the performance of a workload.
#                The score is used to judge the merit of a rule set.
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-06
#
import sys

candidate_avg_bw = float(sys.argv[1])
candidate_avg_var = float(sys.argv[2])
afactor = 0.5
k = 2
objective_model = str(sys.argv[3])


# The scores of this method do not reflect the speed variance.
# To reflect the variance in the score, we can multiply var
# by a factor  and add it to tp:
def afactor_score():
    score = candidate_avg_bw - afactor * candidate_avg_var
    if score > 0:
        return score
    return 0


# coefficient of variation
# To avoid using an arbitrary number as the range limit,we use
# the coefficient of variation (CV), which is calculated as
# k = var/tp.
def cv_score():
    if candidate_avg_var * 1.0 / candidate_avg_bw < k:
        return candidate_avg_bw
    return 0


objective_function = {"cv": cv_score, "afactor": afactor_score}


def get_score(_objective_model="afactor"):
    return objective_function.get(_objective_model)()


if objective_model not in objective_function.keys():
    objective_model = "afactor"
print(get_score(objective_model))
