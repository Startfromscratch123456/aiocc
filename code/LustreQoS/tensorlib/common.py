
import re
import subprocess


def extract_time_from_ping_output(s: str) -> float:
    return float(re.search('(?<=time=)[0-9\.]+', s).group(0))


def get_ping_time(host: str) -> float:
    with subprocess.Popen(["/bin/ping", "-c1", "-w100", host], stdout=subprocess.PIPE) as proc:
        return extract_time_from_ping_output(proc.stdout.read().decode('utf-8'))
