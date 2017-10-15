citron -e "Pen green write: 'Citron: \nsource> \n', resetColor write: (File new: 'serializing.ctr', read), blue write: '\nbenchmark: ', resetColor"
python -m timeit -n 2 -r 50 -s 'import os' 'os.system("ctr serializing.ctr")'

citron -e "Pen green write: 'Perl: \nsource> \n', resetColor write: (File new: 'serializing.pl', read), blue write: '\nbenchmark: ', resetColor"
python -m timeit -n 2 -r 50 -s 'import os' 'os.system("perl serializing.pl")'

citron -e "Pen green write: 'Python: \nsource> \n', resetColor write: (File new: 'serializing.py', read), blue write: '\nbenchmark: ', resetColor"
python -m timeit -n 2 -r 50 -s 'import os' 'os.system("python serializing.py")'

citron -e "Pen green write: 'Ruby: \nsource> \n', resetColor write: (File new: 'serializing.rb', read), blue write: '\nbenchmark: ', resetColor"
python -m timeit -n 2 -r 50 -s 'import os' 'os.system("ruby serializing.rb")'
