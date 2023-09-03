from google.cloud import storage
import elker

def update_and_upload(blob):
    elker.extract('./skindata.csv')
    elker.update('./skindata.csv', './skins.csv')

    blob.upload_from_filename(filename = './skins.csv')

#import functions_framework

#@functions_framework.http
def elker_http(request):
    client = storage.Client()
    bucket = client.get_bucket('elker-bucket')

    update_and_upload(bucket.blob('skins.csv'))
    return 'ok'

if __name__ == '__main__':
    client = storage.Client()
    bucket = client.get_bucket('elker-bucket')

    update_and_upload(bucket.blob('skins.csv'))


