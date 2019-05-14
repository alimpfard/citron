# Citron CMD One-liners

+ List files and folders with separate colors

 - ```citron -e "(File list: '.') each: {:i :f (((f @ 'type') = 'folder') either: {Pen green write: (f @ 'file'), resetColor brk. } or: {Pen write: (f @ 'file'), resetColor brk.} ) run. }"
```
