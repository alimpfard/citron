require 'json'

data = JSON.parse(File.read('generated (1).json'))
stri = JSON.unparse(data)
stri #Ruby's not lazy either!
