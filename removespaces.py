# servicio web para sustituir todos los espacios en blanco a 1 espacio
# en blanco en una cadena de texto
from spyne import Application, rpc, ServiceBase, Unicode
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication


class TransformService(ServiceBase):
    @rpc(Unicode, _returns=Unicode)
    def transform_string(ctx, string):
        # transform the spaces in the string to a single space, but keep the rest of the string
        transformed_string = ""
        i = 0
        while i < len(string):
            if string[i] == " ":
                transformed_string += " "
                i += 1
                while i < len(string) and string[i] == " ":
                    i += 1
            else:
                transformed_string += string[i]
                i += 1
        #if last bit is a space, remove it
        if transformed_string[-1] == " ":
            transformed_string = transformed_string[:-1]
        return transformed_string


app = Application([TransformService], 'transform',
                  in_protocol=Soap11(validator='lxml'),
                  out_protocol=Soap11())

wsgi_application = WsgiApplication(app)


if __name__ == '__main__':
    import logging
    from wsgiref.simple_server import make_server

    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)

    server = make_server('127.0.0.1', 8000, wsgi_application)
    server.serve_forever()
