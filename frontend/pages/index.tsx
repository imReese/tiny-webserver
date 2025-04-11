import { NextPage } from 'next'
import Head from 'next/head'
import Link from 'next/link'

const Home: NextPage = () => {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center">
      <Head>
        <title>Web Server</title>
        <meta name="description" content="A simple web server" />
        <link rel="icon" href="/favicon.ico" />
      </Head>

      <main className="flex flex-col items-center justify-center w-full flex-1 px-20 text-center">
        <h1 className="text-6xl font-bold">
          Welcome to{' '}
          <a className="text-blue-600" href="https://nextjs.org">
            Web Server
          </a>
        </h1>

        <div className="flex flex-wrap items-center justify-around max-w-4xl mt-6 sm:w-full">
          <div className="p-6 mt-6 text-left border w-96 rounded-xl hover:text-blue-600 focus:text-blue-600">
            <Link href="/login">
              <h3 className="text-2xl font-bold">Login &rarr;</h3>
              <p className="mt-4 text-xl">
                Already have an account? Sign in here.
              </p>
            </Link>
          </div>

          <div className="p-6 mt-6 text-left border w-96 rounded-xl hover:text-blue-600 focus:text-blue-600">
            <Link href="/register">
              <h3 className="text-2xl font-bold">Register &rarr;</h3>
              <p className="mt-4 text-xl">
                New to our platform? Create an account here.
              </p>
            </Link>
          </div>
        </div>
      </main>
    </div>
  )
}

export default Home 