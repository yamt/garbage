use anyhow::Context;
use anyhow::Result;
use nokhwa::pixel_format::RgbFormat;
use nokhwa::utils::CameraIndex;
use nokhwa::utils::RequestedFormat;
use nokhwa::utils::RequestedFormatType;
use nokhwa::Buffer;
use nokhwa::Camera;

fn process(frame: &Buffer) -> Result<()> {
    println!(
        "processing a frame of len={} format={}",
        frame.buffer().len(),
        frame.source_frame_format()
    );
    let image = frame.decode_image::<RgbFormat>()?;
    let unixtime_ns = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .expect("unix time")
        .as_nanos();
    image.save(format!("{}.png", unixtime_ns))?;
    Ok(())
}

fn main() -> Result<()> {
    nokhwa::nokhwa_initialize(|granted| {
        println!("granted: {}", granted);
    });
    let index = CameraIndex::Index(0);
    let requested =
        RequestedFormat::new::<RgbFormat>(RequestedFormatType::AbsoluteHighestFrameRate);
    let mut camera = Camera::new(index, requested).context("camera")?;
    camera.open_stream()?;
    println!("camera format {}", camera.camera_format());
    loop {
        let frame = camera.frame()?;
        process(&frame)?;
    }
}
