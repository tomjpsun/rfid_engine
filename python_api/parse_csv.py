import csv
from io import StringIO
import pandas as pd

scsv = """text,with,Polish,non-Latin,letters
1,2,3,4,5,6
a,b,c,d,e,f
gęś,zółty,wąż,idzie,wąską,dróżką,
"""
scsv = """text,with,Polish,non-Latin,letters
1,2,3,4,5,6
a,b,c,d,e,f
"""
data = "col1,col2,col3\na,b,1\na,b,2\nc,d,3"

# parse with csv
f = StringIO(scsv)
reader = csv.reader(f, delimiter=',')
for row in reader:
    print('\t'.join(row))

# parse with pandas
f =  StringIO(data)
df = pd.read_csv(f)
print(df)
