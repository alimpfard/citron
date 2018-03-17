from ipykernel.kernelbase import Kernel
import subprocess, random, socket, re, json

tokenizer_re = re.compile(r'''\'(\\\'|[^\'])*?\'|(\?\>[^(<?)]*?\<\?)|\s+''')

class CitronKernel(Kernel):
    implementation = 'Citron'
    implementation_version = '1.0'
    language = 'citron'
    language_version = '0.0.7.4-boehm-gc'
    language_info = {
            'name': 'Citron',
            'mimetype': 'text/plain',
            'file_extension': '.ctr',
            }
    banner = 'Citron kernel'

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.port = str(random.randint(5000, 6000))
        self.ctr_server = subprocess.Popen(['citron', '-s', self.port])

    def send_ctr_request(self, api_method, code=None):
        connected = False
        while not connected:
            try:
                self.ctr_socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM, 0)
                self.ctr_socket.connect(('localhost', int(self.port), 0, 0))
                connected = True
            except Exception as e:
                raise e
        req = "Array < '%s' %s" % (api_method, ('; ' + (code if code else '')))
        with open(api_method.replace(':','_')+'.in', 'w') as f:
            f.write(req)
        self.ctr_socket.send(bytes(req, 'utf-8'))
        v = b''
        while True:
            b = self.ctr_socket.recv(10240)
            if not b: break
            v += b
        self.ctr_socket.close()
        with open(api_method.replace(':','_')+'.out', 'wb') as f:
            f.write(v)
        return v.decode()


    def do_execute(self, code, silent, store_history=True, user_expressions=None,
            allow_stdin=False):
        value = "?>%s<?" % code
        [value, err] = json.loads(self.send_ctr_request('evaluate:', value))
        # with open('err.v', 'w') as f:
        #     f.write(err)
        err = json.loads(err.strip() or '""')
        if not silent:
            stream_content = {'name': 'stdout' if not err[0] else 'stderr', 'text': value if not err[0] else err[3]}
            self.send_response(self.iopub_socket, 'stream', stream_content)
        return {'status': 'ok',
                'ename': '' if not err[0] else err[2],
                'evalue': '' if not err[0] else err[3],
                'traceback': [] if not err[0] else err[1],
                # The base class increments the execution count
                'execution_count': self.execution_count,
                'user_expressions': {'names': self.send_ctr_request('allTheNames:', '_')},
               } if not err[0] else {
                'status': 'error',
                'ename': err[2],
                'evalue': err[3],
                'traceback': err[1],
                'execution_count': self.execution_count,
                }

    def do_complete(self, code, cursor_pos):
        cst = cursor_pos
        while cursor_pos < len(code) and re.match(r'[^\s\\\']', code[cursor_pos]): cursor_pos += 1
        token = tokenizer_re.split(code[:cursor_pos])[-1]
        token = (token.replace('\'', '\\\''))
        fs = self.send_ctr_request('allTheNames')
        # with open('comp.fs', 'w') as f:
        #     f.write(fs)
        if not fs.startswith('Array ') and not fs.startswith('['): return {
            'status': 'ok',
            'matches': [],
            'cursor_start': cst,
            'cursor_end': cst,
        }
        if fs[0] == '[': fs = fs[1:]
        else: fs = fs[9:]
        value = fs[:-1].split("' ; '")
        # with open('comp.l', 'w') as f:
        #     f.write(str(value))
        return {
            'metadata': {},
            'status': 'ok',
            'matches': [i for i in value if i.startswith(token)],
            'cursor_start': cst - len(token),
            'cursor_end': cursor_pos,
            'payload': {},
        }

    def do_shutdown(self, restart):
        self.ctr_server.kill()
        if restart:
            self.port = str(random.randint(5000, 6000))
            self.ctr_server = subprocess.Popen(['citron', '-s', self.port])


if __name__ == '__main__':
    from ipykernel.kernelapp import IPKernelApp
    IPKernelApp.launch_instance(kernel_class=CitronKernel)
