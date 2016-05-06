Tratamento de hazards:
----
#####Hazard de dados
Surge quando uma instrução depende de uma atualização de dados de alguma instrução anterior que ainda está no pipeline. A maioria desses hazards pode ser corrigida com forwarding. 

Tipos de hazard de dados:
 * (RAW) Read after write
 * (WAR) Write after read
 * (WAW) Write after write

Para avaliar os hazards modificaremos o código do simulador. Para detectar, é necessário armazenar os estados do pipeline e quais dados já foram atualizados, através de vetores auxiliares globais no arquivo *mips_isa.cpp*. Além disso é preciso saber quais "hazards" são corrigíveis com forwarding e desconsiderá-los. Para mais referências sobre esses hazards: (wikipedia)[https://en.wikipedia.org/wiki/Hazard_(computer_architecture)].

#####Hazard de controle
Ocorre quando o instruction fetch carrega no pipeline uma instrução que não será executada. Isto ocorre quando o fluxo de execução é imprevisível, por causa de *branches*.

* Sem nenhum tratamento de hazard: o processador deve, toda vez que executa um branch que talvez será satisfeito, incluir duas bolhas no pipeline. Isto é bom se o código raramente satisfaz seus branches, mas na prática não é bem assim que ocorre.
* Tratamento por branch prediction estático: o processador sempre segue uma regra fixa. Um exemplo (que foi o implementado no projeto) é sempre admitir que o branch será satisfeito. O processador só descobrirá se sua previsão foi correta no estágio lógico (ALU) do pipeline. Se ele acerta, nenhuma correção precisa ser feita; se ele erra, é preciso pular para o fluxo de não satisfação, transformando em bolha aquelas instruções incorretamente computadas no pipeline.
* Tratamento por branch prediction dinâmico: o processador guarda um histórico do comportamento do PC para fazer suas previsões de fluxo de execução. Na prática muito tempo de computação é gasto em laços, os quais tem um comportamento periódico parcialmente previsível, tornando esta técnica bastante acurada segundo a literatura.

Para a detecção de hazards de controle é preciso que se admita um dos tratamentos de branch prediction para cada experimento. Logo foi necessário modificar o código do arquivo do simulador *mips_isa.cpp* e implementar cada um dos comportamentos descritos acima econtar quando eles acusariam hazards e quanta penalidade (ciclos gastos em bolhas) cada um tem.

Referências
----
1. http://www.ic.unicamp.br/~lucas/teaching/mc723/2016-1/p2.html
2. https://en.wikipedia.org/wiki/Hazard_(computer_architecture)





