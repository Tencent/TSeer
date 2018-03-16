
class FileNotExist(Exception):
    def __init__(self, filename):
        self.filename = filename

    def __str__(self):
        return repr("File %s not exists" %(self.filename))

if __name__ == '__main__':
    raise FileNotExist('hosts')
