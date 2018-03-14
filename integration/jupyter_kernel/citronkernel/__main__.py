from ipykernel.kernelapp import IPKernelApp
from .citronkernel import CitronKernel

IPKernelApp.launch_instance(kernel_class=CitronKernel)
