// a dumb port of C++ version:
// https://github.com/yamt/garbage/tree/master/cv

use anyhow::anyhow;
use anyhow::Result;
use opencv::highgui;
use opencv::prelude::Mat;
use opencv::prelude::MatTraitConst;
use opencv::prelude::VideoCaptureTrait;
use opencv::videoio;
use tracing::info;

fn main() -> Result<()> {
    /*
     * Note: opencv doesn't have an API to list camera devices.
     * https://github.com/opencv/opencv/issues/4269
     * While ID 0 seems to work for many cases, it doesn't always work.
     * For example, while I can successfully open ID 0 on my macbook,
     * it returns an empty frame for some reasons. The front camera
     * on the macbook seems working if I specify ID 1.
     */
    let camera_id = match std::env::var("CAMERA_ID") {
        Ok(camera_id_str) => camera_id_str.parse()?,
        _ => 0,
    };
    let mut cap = videoio::VideoCapture::new(camera_id, videoio::CAP_ANY)?;
    let mut frame = Mat::default();
    loop {
        cap.read(&mut frame)?;
        if frame.empty() {
            info!("empty frame");
            break Err(anyhow!("empty frame"));
        }
        highgui::imshow("live", &frame)?;
        highgui::wait_key(5)?;
    }
}
