import paramiko
import re
import time
import socket

def captura_rssi(estacao):
    try:
        ssh_client = paramiko.SSHClient()
        ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh_client.connect(estacao, username='admin', password='admin',timeout = 20)
        
        stdin,stdout,stderr=ssh_client.exec_command('show interfaces-state interface LnRadio ln-status last-rx-packet last-rssi | tab')
        output = stdout.readlines()
    except (socket.error):
        print ("Perda da estacao " + estacao)
        output = ['0']
    for items in output:
        #print(items)
        return int((-1)*int(re.findall('\d+', items)[0]))


nome_arquivo = "rssi.txt"
arquivo_captura = open(nome_arquivo, 'w')
#arquivo_captura.write("dados_rssi \n")
estacao_erb = '192.168.26.130';
estacao_remota = '192.168.26.131';
rssi_remota = captura_rssi(estacao_remota)
rssi_erb = captura_rssi(estacao_erb)
arquivo_captura.write(str(rssi_erb)+" "+str(rssi_remota))
arquivo_captura.close()

