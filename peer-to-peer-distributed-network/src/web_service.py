from spyne import Application, ServiceBase, rpc, Unicode
from spyne.protocol.soap import Soap11
import logging
from spyne import Application, ServiceBase, rpc, Unicode
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication
from datetime import datetime
from wsgiref.simple_server import make_server

class DateTimeService(ServiceBase):
    @rpc(_returns=Unicode)
    def get_datetime(ctx):
        return datetime.now().strftime('%d/%m/%Y %H:%M:%S')

soap_app = Application(
    services=[DateTimeService],
    tns='spyne.examples.datetime',
    in_protocol=Soap11(validator='lxml'),
    out_protocol=Soap11()
)

wsgi_app = WsgiApplication(soap_app)

if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)
    logging.info("Listening on http://127.0.0.1:5500; WSDL at: http://127.0.0.1:5500/?wsdl")
    server = make_server('127.0.0.1', 5500, wsgi_app)
    server.serve_forever()
