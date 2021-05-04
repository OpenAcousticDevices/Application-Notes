import re
import csv

from os import listdir, stat
from os.path import isfile, join
from datetime import datetime, timezone, timedelta

directory = "."

COMMENT_START = 0x38
COMMENT_LENGTH = 0x180

files = [f for f in listdir(directory) if isfile(join(directory, f)) and ".WAV" in f and "._" not in f]

with open("comments.csv", "w", newline="") as csvfile:

  csvWriter = csv.writer(csvfile, delimiter=",")
    
  csvWriter.writerow(["Index", "File", "Time", "Battery (V)", "Temperature (C)", "Comment"])
    
  for i, fi in enumerate(sorted(files)):
    
    if stat(join(directory, fi)).st_size > 0:

      with open(join(directory, fi), "rb") as f:

        print("Processing " + fi)
          
        # Read the comment out of the input file

        f.seek(COMMENT_START)

        comment = f.read(COMMENT_LENGTH).decode("ascii").rstrip("\0")

        # Read the time and timezone from the header

        ts = re.search(r"(\d\d:\d\d:\d\d \d\d/\d\d/\d\d\d\d)", comment)[1]

        tz = re.search(r"\(UTC([-|+]\d+)?:?(\d\d)?\)", comment)

        hrs = 0 if tz[1] is None else int(tz[1])

        mins = 0 if tz[2] is None else -int(tz[2]) if hrs < 0 else int(tz[2])

        timestamp = datetime.strptime(ts, "%H:%M:%S %d/%m/%Y")
          
        timestamp = timestamp.replace(tzinfo=timezone(timedelta(hours=hrs, minutes=mins)))

        # Read the battery voltage and temperature from the header

        battery = re.search(r"(\d\.\d)V", comment)[1]

        temperature = re.search(r"(-?\d+\.\d)C", comment)[1]

        # Print the output row
      
        csvWriter.writerow([i, fi, timestamp.isoformat(), battery, temperature, comment])
