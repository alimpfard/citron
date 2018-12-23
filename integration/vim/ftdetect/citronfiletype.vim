let s:current_fileformats = ''
let s:current_fileencodings = ''

function! s:citronfiletype_pre(type)
    let s:current_fileformats = &g:fileformats
    let s:current_fileencodings = &g:fileencodings
    set fileencodings=utf-8 fileformats=unix
    let &l:filetype = a:type
endfunction

" restore
function! s:citronfiletype_post()
    let &g:fileformats = s:current_fileformats
    let &g:fileencodings = s:current_fileencodings
endfunction

au BufNewFile *.ctr setfiletype citron | setlocal fileencoding=utf-8 fileformat=unix
au BufRead *.ctr call s:citronfiletype_pre("citron")
au BufReadPost *.ctr call s:citronfiletype_post()

