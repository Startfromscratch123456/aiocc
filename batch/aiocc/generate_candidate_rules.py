# description:   generate candidate rules
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-14
#
import sys
import csv
import os.path

# "${TESTED_RULES_DIR}/${BEST_RULE}/summary" "${CANDIDATES_DIR}" $next_rule_sequence $CURRENT_RULE "$EPOCH_FINISHED_RULES_FILE"

# consts
debug = 0
used_times_column_index = 9

if len(sys.argv) < 4:
    print("Usage:", sys.argv[0],
          "input_rule_file output_dir next_rule_sequence [working_on_rule] [list_file_of_excluded_rules]")
    exit(2)

input_rule_file = sys.argv[1]
output_dir = sys.argv[2]
next_rule_sequence = int(sys.argv[3])
next_rule_sn_file = sys.argv[4]
working_on_rule = -1
excluded_rules = []

if len(sys.argv) >= 6:
    working_on_rule = int(sys.argv[5])
    list_file_of_excluded_rules = sys.argv[6]
    if os.path.isfile(list_file_of_excluded_rules):
        f = open(list_file_of_excluded_rules, 'r')
        while True:
            s = f.readline()
            if s == '':
                break
            excluded_rules.append(int(s))
    if debug > 0:
        print('excluded_rules: ', excluded_rules)

# Load excluded rules from file. Excluded rules will be ignored.
# They are used by the Remy method.

fields = []
# Remy used 20 for upper limit for b with a max_window = 256.
# Our mrif_upper_limit is 30, therefore we should use
# mrif_upper_limit / ( 256 / 20 ) * 100
# max rpcs in flight (MRIF)
mrif_upper_limit = 30
b100_upper_limit = mrif_upper_limit * 20 * 100 / 256
b100_lower_limit = -b100_upper_limit
fields.append(dict(name="m100", column_index=6, lower_limit=1, upper_limit=200, delta_gran=5, exhaust_search_step=4))
fields.append(dict(name="b100", column_index=7, lower_limit=b100_lower_limit, upper_limit=b100_upper_limit, delta_gran=30,
                   exhaust_search_step=4))
fields.append(dict(name="tau", column_index=8, lower_limit=0, upper_limit=70000, delta_gran=500, exhaust_search_step=6))

rule_no = 0
rules = []
busiest_rule_used_times = 0
row_index = 0


def write_rule_file():
    global next_rule_sequence
    global rules
    global output_dir
    global rule_to_tweak
    global working_on_rule
    global mrif_update_rate
    filename = output_dir + '/' + str(next_rule_sequence)
    f = open(filename, 'w')
    next_rule_sequence += 1
    print("%d,%d" % (rule_no, mrif_update_rate), file=f)
    row_index = 0
    for rule in rules:
        s = ""
        if row_index == working_on_rule:
            for col in rule_to_tweak:
                if s != "":
                    s += ","
                s += str(col)
        else:
            for col in rule:
                if s != "":
                    s += ","
                s += str(col)
        print(s, file=f)
        row_index += 1


# read in current rules
#best rules summary file
with open(input_rule_file, 'r') as qos_rule_file:
    qos_rule_file_content = csv.reader(qos_rule_file)
    for row in qos_rule_file_content:
        # first line is rule_no
        if rule_no == 0:
            rule_no = int(row[0])
            mrif_update_rate = int(row[1])
            continue

        # read in rules
        new_rule = []
        for col in row:
            new_rule.append(int(col))
        if debug >= 2:
            print("Read in rule", new_rule)

        rules.append(new_rule)
        if not row_index in excluded_rules and new_rule[used_times_column_index] > busiest_rule_used_times:
            busiest_rule_used_times = new_rule[used_times_column_index]
            busiest_rule = new_rule
            busiest_rule_id = row_index

        row_index += 1

if debug > 0:
    print('Busiest rule index: %d' % (busiest_rule_id))

if working_on_rule != -1:
    rule_to_tweak = list(rules[working_on_rule])
else:
    working_on_rule = busiest_rule_id
    rule_to_tweak = list(busiest_rule)


def gen_rules_using_field(field_id):
    if field_id >= len(fields):
        write_rule_file()
        return

    field = fields[field_id]
    if debug >= 1:
        print("Working on field %s" % (field['name']))

    b = field['upper_limit']
    a = field['lower_limit']
    gran = field['delta_gran']
    step = field['exhaust_search_step']
    scale_factor = pow(float(b - a) / gran, 1.0 / step)
    if debug >= 1:
        print("scale_factor: %f" % (scale_factor))
    init_val = rules[working_on_rule][field['column_index']]

    if init_val > b or init_val < a:
        print("Warning: out of range of %s value %d, skip processing this field" % (field['name'], init_val),
              file=sys.stderr)
        gen_rules_using_field(field_id + 1)
        return

    test_range = max(b - init_val, init_val - a, 0)

    delta = gran
    while delta <= test_range:
        if debug >= 1:
            print("Field %s delta: %f" % (field['name'], delta))
        if init_val - delta >= a:
            rule_to_tweak[field['column_index']] = int(init_val - delta)
            gen_rules_using_field(field_id + 1)
        if init_val + delta <= b:
            rule_to_tweak[field['column_index']] = int(init_val + delta)
            gen_rules_using_field(field_id + 1)
        delta *= scale_factor

gen_rules_using_field(0)
with open(next_rule_sn_file, 'w+') as f:
    print("% d, % d" % (next_rule_sequence, working_on_rule), file=f)