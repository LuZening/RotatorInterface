import numpy as np

# f: base freq
f = 50 # Hz

# fm: modulation freq
fm  = 2000 # Hz
# fc: carrier freq
fc = fm # Hz

# alpha: modulation depth
alpha = 1
# N: length of table
N = int(fm / f / 2) # only contains half period


Phi = np.linspace(0, np.pi, N)
Vals = np.sin(Phi)

tableDuty = alpha * Vals
tableUS = np.round(tableDuty * 1000000 / fc).astype(np.int16)
print(tableDuty)
print(f'Table length: {N}')
sOut = ', '.join(tableUS.astype(str))
print(sOut)










