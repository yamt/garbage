
# query statistics of a youtube video, using google api

# create a project and api key. set it to the environment variable "API_KEY"
# https://developers.google.com/youtube/registering_an_application
#
# enable youtube v3 api for the project
# https://console.developers.google.com/apis/api/youtube.googleapis.com/overview?project=23442396519
#
# install clienty library
# pip install --user google-api-python-client
#
# set the environment variable VIDEO_ID
# https://www.youtube.com/watch?v=<__VIDEO_ID_is_here__>

import os
import time
import datetime

from apiclient.discovery import build

api_key = os.getenv('API_KEY')
video_id = os.getenv('VIDEO_ID')

yt = build("youtube", "v3", developerKey=api_key)

# https://developers.google.com/youtube/v3/docs/videos/list
op = yt.videos().list(part='statistics', id=video_id)

def extract_stats(result):
    # result here is something like
    # {'kind': 'youtube#videoListResponse', 'etag': 'E6-Bzb4X_fDK5xKsNzx6LWpuOdY', 'items': [{'kind': 'youtube#video', 'etag': 'CTSoVMDunhrUb-G04W0MqgsMtFI', 'id': 'drvH4XbZoPs', 'statistics': {'viewCount': '281260', 'likeCount': '2271', 'dislikeCount': '29', 'favoriteCount': '0', 'commentCount': '50'}}], 'pageInfo': {'totalResults': 1, 'resultsPerPage': 1}}

    return result['items'][0]['statistics']

while True:
    result = op.execute()
    stats = extract_stats(result)

    now = datetime.datetime.now()

    print("{}, {}".format(now, stats['viewCount']))

    # it seems that the statistics are updated every 5 minutes.
    time.sleep(5 * 60)
