-- pick and print a random line from the text file

-- the primary purpose of this program is to help you to decide
-- which programming language to use for your project.

-- i wrote the first version of this program in python.
-- i then re-wrote it in haskell, following the decision made by
-- the first version of this program.

import System.Random
import System.Environment
import Data.Time.Clock.POSIX

-- roll our own random seeding to avoid getpid,
-- which causes a link-time error on wasi
rand :: (Int, Int) -> IO [Int]
rand (a, b) =
    do
        now <- getPOSIXTime
        let
            seed :: Integer = floor $ (1000 *) $ toRational now
            stdgen = mkStdGen $ fromIntegral seed
            rs = randomRs (a, b) stdgen
        return rs

pick :: [String] -> IO String
pick xs =
    do
        let len = length xs
        rs <- rand (0, len - 1)
        case rs of
            [] -> return "" -- just to avoid partial func
            (i:_) -> return $ xs !! i

main :: IO ()
main =
    do
        args <- getArgs
        c <- readFile (args !! 0)
        choice <- pick $ lines c
        putStrLn choice
