# infra_software_memory
Atividade para obtenção de nota parcial da cadeira "Infraestrutura de Software"

## Como clonar o repositório:
```bash
$ git clone https://github.com/ElderLamarck/infra_software_memory.git
$ cd infra_software_memory
```

## Como utilizar o programa:
O código simula como funciona a munipulação de uma memoria virtual, esse programa irá abrir um arquivo de texto que possui instruções
e a partir delas iremos converter o valor de instrução para binário extrair a page e o offset dessa instrução, em seguida iremos buscar
a pagina em um arquivo chamado BACKING_STORE.bin, iremos salvar essa página na memoria e por fim printar o valor correspondente a instrução.

### Para rodar e testar clone o repositório, entre no diretório que foi criado e execute os comandos
```bash
$ make
$ make test1
```
esse comando deverá criar um arquivo chamado correct.txt e que os ultimos valores deverão ser:

Virtual address: 45563 Physical address: 6395 Value: 126
Virtual address: 12107 Physical address: 6475 Value: -46
Number of Translated Addresses = 1000
Page Faults = 538
Page Fault Rate = 0.538
TLB hits = 54
TLB rate = 0.054

existem 5 casos de teste pois é necessario dizer 2 tipos de substituição FIFO e RLU, tanto para a substituição da memoria fisica quanto
para a estrutura da TLB.

test1 - fifo fifo
test2 - lru fifo
test3 - fifo lru
test4 - lru lru
test5 - caso de erro, não passar argumentos
