##### Universidade do Estado do Rio de Janeiro
##### Disciplina: Aspectos Práticos em Ciência da Computação II  → Sistemas Reativos
##### Projeto Final Reativos  - 2016.2

##### Alunos: Camila Eleutério Gusmão e
##### Renato Domingues Carneiro Júnior
---
#### Ideias de Projeto : Jogo SDL + Arduino

A ideia consiste em aprimorarmos o jogo em SDL criando para este um controlador utilizando Arduino.

##### Melhorias para o jogo:
* Permitir que ao final do jogo o usuário possa escolher entre jogar novamente ou sair do jogo [C]
* Criação de tela inicial do jogo contendo: [C]
    * Instruções;
    * Recorde alcançado;
    * Nível de dificuldade;
* Criação de esquema de pontuação [R]
* Exibição do tempo de jogo [C]
* Gerência de vidas para o usuário(Atualmente o jogo termina quando o jogador é atingido por algum tiro ou colide com um inimigo). O objetivo é que ela tenha inicialmente 3 vidas.  [R]
* Bonificação: Vidas, invencibilidade por alguns segundos [R]
* Escolha do nível de dificuldade, que será dado por inclusão gradativa de inimigos [C]
* Pausar jogo [R]
* Sons [R]

##### Criação do Controlador:
O objetivo é criar um joystick para o jogo, sendo possível:
 * Mover nave para a direita; [C]
 * Mover nave para a esquerda; [C]
 * Atirar; [C]
 * Controlar opções do jogo: Pausar, sair, retomar, escolher opções do menu inicial. [R]
